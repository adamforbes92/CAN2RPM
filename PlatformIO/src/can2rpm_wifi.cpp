#include "can2rpm_defs.h"
#include "can2rpm_vers.h"
#include "power_manager.h"
#include "wifi_manager.h"
#include "ota_manager.h"

static uint32_t parseHexCanId(const String& text) {
    String s = text;
    s.trim();
    if (s.startsWith("0x") || s.startsWith("0X")) {
        s = s.substring(2);
    }
    if (s.length() == 0) {
        return AFTERMARKET_ID;
    }
    return (uint32_t)strtoul(s.c_str(), nullptr, 16);
}

static int32_t parseMultiplierFx(const String& text) {
    float f = text.toFloat();
    int32_t fx = (int32_t)(f * (float)aftermarketMultiplierScale);
    if (fx < -1000000) fx = -1000000;
    if (fx > 1000000)  fx = 1000000;
    return fx;
}

// ─────────────────────────────────────────────────────────────────
// Internal helper – serialise a JsonDocument and send response
// ─────────────────────────────────────────────────────────────────
static void sendJson(AsyncWebServerRequest* req, int code, const JsonDocument& doc) {
    String out;
    serializeJson(doc, out);
    req->send(code, "application/json", out);
}

static void resetDecodedRpmState() {
    vehicleRPM = 0;
    rpmTruth = false;
}

// ─────────────────────────────────────────────────────────────────
// generalCallback – replaces ESPUI generalCallback
// Called per POST /api/control with a key/value pair from the UI.
// ─────────────────────────────────────────────────────────────────
void generalCallback(const String& key, const String& value) {
#if serialDebugWifi
    Serial.print("Control: "); Serial.print(key);
    Serial.print(" = ");       Serial.println(value);
#endif

    if (key == "hasNeedleSweep")      hasNeedleSweep  = (value == "true" || value == "1");
    if (key == "sweepSpeed")          sweepSpeed      = (uint8_t)value.toInt();
    if (key == "clusterRPMLimit")     clusterRPMLimit = (uint16_t)value.toInt();
    if (key == "maxRPM")              maxRPM          = (uint16_t)value.toInt();
    if (key == "tempDiagTest")        tempDiagTest    = (value == "true" || value == "1");
    if (key == "tempRPM")             tempRPM         = (uint16_t)value.toInt();
    if (key == "ecuMode") {
        uint8_t newMode = (uint8_t)((value.toInt() == 1) ? 1 : 0);
        if (ecuMode != newMode) {
            ecuMode = newMode;
            resetDecodedRpmState();
        }
    }
    if (key == "aftermarketCanIdHex") {
        uint32_t newCanId = parseHexCanId(value);
        if (aftermarketCanId != newCanId) {
            aftermarketCanId = newCanId;
            resetDecodedRpmState();
        }
    }
    if (key == "aftermarketByteLow") {
        uint8_t newByteLow = (uint8_t)constrain(value.toInt(), 0, 7);
        if (aftermarketByteLow != newByteLow) {
            aftermarketByteLow = newByteLow;
            resetDecodedRpmState();
        }
    }
    if (key == "aftermarketByteHigh") {
        uint8_t newByteHigh = (uint8_t)constrain(value.toInt(), 0, 7);
        if (aftermarketByteHigh != newByteHigh) {
            aftermarketByteHigh = newByteHigh;
            resetDecodedRpmState();
        }
    }
    if (key == "aftermarketMultiplier") aftermarketMultiplierFx = parseMultiplierFx(value);
    if (key == "aftermarketAddition") aftermarketAddition = (int16_t)constrain(value.toInt(), -1000, 1000);
}

// ─────────────────────────────────────────────────────────────────
// extendedCallback – replaces ESPUI extendedCallback
// Called per POST /api/action with a named action from the UI.
// ─────────────────────────────────────────────────────────────────
void extendedCallback(const String& action) {
#if serialDebugWifi
    Serial.print("Action: "); Serial.println(action);
#endif

    if (action == "needleSweep") {
        tempNeedleSweep = true;                  // handled in main loop
    } else if (action == "resetClusterRPM") {
        clusterRPMLimit = 7000;
    } else if (action == "resetRPMScaling") {
        maxRPM = 230;
    }
}

void connectWifi() {
    DEBUG_PRINTLN("Creating access point...");

    // Common SoftAP + mDNS + LittleFS front-end (reachable at can2rpm.local).
    wifimgr_config_t wcfg = wifiDefaultConfig();
    wcfg.hostName  = wifiHostName;   // SoftAP SSID + WiFi hostname
    wcfg.mdnsName  = "can2rpm";      // -> http://can2rpm.local
    wcfg.fwVersion = FW_VERSION;     // recovery page only; index.html bakes its own
    // MUST precede wifiManagerInit(): that mounts the web-UI filesystem via
    // otaFsMountSafe(), so ota_manager has to be configured first or a failed
    // mount passes silently.
    ota_config_t ocfg = otaDefaultConfig();
    ocfg.fwVersion  = FW_VERSION;
    ocfg.product    = "Can2RPM";
    ocfg.githubRepo = "adamforbes92/CAN2RPM"; // Releases/ + releases.json for "Check for updates"
    otaManagerInit(&ocfg);

    wifiManagerInit(&wcfg);
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // lower TX power to cut regulator heat
    DEBUG_PRINTLN("Access point ready: " wifiHostName);
}

void checkConnections() {
    DEBUG_PRINTF("WiFi clients: %d\n", WiFi.softAPgetStationNum());
}

void setupUI() {
    // LittleFS is mounted (guarded) by wifiManagerInit(); just register routes below.

    // ── Shared OTA + Home WiFi routes FIRST ──────────────────────
    // ota_manager's first route carries the filter that notes web activity for
    // every request (otaWebClientActive()), and /api/wifi/sta must precede any
    // /api/wifi... route of our own. Registers /api/ota, /api/ota/fs,
    // /api/ota/info, /api/ota/check, /api/ota/fsinfo, /api/ota/fsdiag and
    // /api/wifi/sta, /api/wifi/sta/reset, /api/wifi/scan.
    otaManagerAttach(server);
    wifiManagerAttachSta(server);

    // ── GET /api/settings ────────────────────────────────────────
    // Returns all persistent configuration values.
    server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["hasNeedleSweep"]  = hasNeedleSweep;
        doc["sweepSpeed"]      = sweepSpeed;
        doc["maxRPM"]          = maxRPM;
        doc["clusterRPMLimit"] = clusterRPMLimit;
        doc["tempRPM"]         = tempRPM;
        doc["tempDiagTest"]    = tempDiagTest;
        doc["ecuMode"]         = ecuMode;
        doc["aftermarketCanIdHex"] = String(aftermarketCanId, HEX);
        doc["aftermarketByteLow"] = aftermarketByteLow;
        doc["aftermarketByteHigh"] = aftermarketByteHigh;
        doc["aftermarketMultiplier"] = (float)aftermarketMultiplierFx / (float)aftermarketMultiplierScale;
        doc["aftermarketAddition"] = aftermarketAddition;
        doc["FW_VERSION"]      = FW_VERSION;
        sendJson(req, 200, doc);
    });

    // ── GET /api/status ──────────────────────────────────────────
    // Returns live vehicle data and computed output frequency.
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req) {
        JsonDocument doc;
        uint16_t rpm = (uint16_t)vehicleRPM;
        doc["vehicleRPM"]   = rpm;
        doc["hasCAN"]       = canHealthy;
        doc["rpmTruth"]     = rpmTruth;
        doc["tempDiagTest"] = tempDiagTest;
        doc["tempRPM"]      = tempRPM;
        doc["ecuMode"]      = ecuMode;

        // Compute current output Hz (same logic as main loop)
        long srcRPM  = tempDiagTest ? (long)tempRPM : (long)rpm;
        long outHz   = (clusterRPMLimit > 0)
                         ? map(srcRPM, 0, clusterRPMLimit, 0, maxRPM)
                         : 0;
        doc["outputHz"] = (uint16_t)constrain(outHz, 0, (long)maxRPM);
        sendJson(req, 200, doc);
    });

    // ── POST /api/control ────────────────────────────────────────
    // Body: { "key": "<setting>", "value": <val> }
    server.addHandler(new AsyncCallbackJsonWebHandler("/api/control",
        [](AsyncWebServerRequest* req, JsonVariant& body) {
            generalCallback(body["key"].as<String>(), body["value"].as<String>());
            req->send(200, "application/json", "{\"ok\":true}");
        }
    ));

    // ── POST /api/action ─────────────────────────────────────────
    // Body: { "action": "<name>" }
    server.addHandler(new AsyncCallbackJsonWebHandler("/api/action",
        [](AsyncWebServerRequest* req, JsonVariant& body) {
            extendedCallback(body["action"].as<String>());
            req->send(200, "application/json", "{\"ok\":true}");
        }
    ));

    // ── "/" (UI, or the recovery page when the filesystem holds no usable UI)
    //    + static files with no-cache revalidation ─────────────────
    wifiManagerAttachStatic(server);

    server.begin();
    DEBUG_PRINTLN("Web server started on port 80");
}

// ----------------------------------------------------------------------------
// power_manager integration (universal reduced-power module)
// ----------------------------------------------------------------------------
// These override the weak hooks in power_manager.cpp. The device stays fully
// awake while ANY client is associated to the AP. Once the last client leaves,
// the manager's idle timer runs, then turns the radio off and drops the CPU
// clock. A power-cycle (ignition off/on) brings WiFi back automatically.

bool powerIsBusy()
{
  // ... or a browser has hit us in the last 30 s (a phone on the home router
  // in bridge mode is not an AP station).
  return WiFi.softAPgetStationNum() > 0 || otaInProgress() || otaWebClientActive();
}

// ACTIVE -> REDUCED: close the web server cleanly before the radio drops.
void powerOnEnterReduced()
{
  server.end();
  wifiManagerStopAP();
}

// REDUCED -> ACTIVE: bring the AP + mDNS and web server back. Routes are already
// registered (no need to re-run setupUI()), so we only restart the radio
// and the listener.
void powerOnExitReduced()
{
  wifiManagerStartAP();
  server.begin();
}
