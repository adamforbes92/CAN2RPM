#include "can2rpm_defs.h"
#include "can2rpm_vers.h"

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

    WiFi.setHostname(wifiHostName);
      WiFi.setTxPower(WIFI_POWER_8_5dBm);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1),
                      IPAddress(192, 168, 1, 1),
                      IPAddress(255, 255, 255, 0));
    WiFi.softAP(wifiHostName);
    WiFi.setSleep(false);   // disable modem sleep for responsive UI
    DEBUG_PRINTLN("Access point ready: " wifiHostName);
}

void checkConnections() {
    DEBUG_PRINTF("WiFi clients: %d\n", WiFi.softAPgetStationNum());
}

void setupUI() {
    if (!LittleFS.begin(true)) {
        DEBUG_PRINTLN("LittleFS: mount failed");
    }

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

    // ── GET /api/ota/info ────────────────────────────────────────
    server.on("/api/ota/info", HTTP_GET, [](AsyncWebServerRequest* req) {
        JsonDocument doc;
        const uint32_t flashBytes = ESP.getFlashChipSize();
        const uint32_t flashMB = flashBytes / (1024UL * 1024UL);

        doc["board"]    = ESP.getChipModel();
        doc["hardware"] = String("rev ") + String((uint32_t)ESP.getChipRevision()) +
                          String(", ") + String((uint32_t)ESP.getChipCores()) + String(" cores, ") +
                          String((uint32_t)ESP.getCpuFreqMHz()) + String("MHz, ") +
                          String((uint32_t)flashMB) + String("MB flash");
        doc["sdk"]      = ESP.getSdkVersion();
        doc["mac"]      = WiFi.softAPmacAddress();
        doc["version"]  = FW_VERSION;
        sendJson(req, 200, doc);
    });

    // ── POST /api/ota/upload ─────────────────────────────────────
    // Completion handler – send result and reboot on success
    server.on("/api/ota/upload", HTTP_POST,
        [](AsyncWebServerRequest* req) {
            bool ok = !Update.hasError();
            req->send(ok ? 200 : 500, "application/json",
                ok ? "{\"message\":\"Update successful! Rebooting...\"}"
                   : "{\"message\":\"Update failed\"}");
            if (ok) { delay(500); ESP.restart(); }
        },
        // Upload handler – receives the binary in chunks
        [](AsyncWebServerRequest* req, const String& filename,
           size_t index, uint8_t* data, size_t len, bool final) {
            if (!index) {
                if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
                    DEBUG_PRINTLN("OTA: begin failed");
                }
            }
            if (Update.write(data, len) != len) {
                DEBUG_PRINTLN("OTA: write error");
            }
            if (final && !Update.end(true)) {
                DEBUG_PRINTLN("OTA: end failed");
            }
        }
    );

    // ── Static web UI from LittleFS ──────────────────────────────
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.begin();
    DEBUG_PRINTLN("Web server started on port 80");
}
