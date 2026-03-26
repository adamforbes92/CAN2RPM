#pragma once

#include <Arduino.h>
#include "driver/twai.h"
#include "driver/ledc.h"
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Update.h>

// ─────────────────────────────────────────────────────────────────
// Debug flags  (0 = off, 1 = on)
// ─────────────────────────────────────────────────────────────────
#define ChassisCANDebug  1   // print raw CAN frames to Serial
#define serialDebug      1   // general Serial talkback
#define serialDebugWifi  1   // WiFi / API Serial talkback

#define wifiHostName  "CAN2RPM"
#define eepRefresh    5000     // EEPROM save interval (ms)

#define pinRX_CAN   17   // SN65HVD230 CAN_RX
#define pinTX_CAN   16   // SN65HVD230 CAN_TX
#define pinCoil     26   // coil trigger output (RPM)
#define onboardLED   2   // on-board LED

#define baudSerial  115200

#define rpmDelay  20   // RPM output update interval (ms)

#define MOTOR1_ID       0x280
#define AFTERMARKET_ID  0x1001

#if serialDebug
  #define DEBUG_PRINT(x)        Serial.print(x)
  #define DEBUG_PRINTLN(x)      Serial.println(x)
  #define DEBUG_PRINTF(fmt,...) Serial.printf(fmt, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(fmt,...)
#endif

#if ChassisCANDebug
  #define CAN_DEBUG_PRINT(x)        Serial.print(x)
  #define CAN_DEBUG_PRINTLN(...)    Serial.println(##__VA_ARGS__)
  #define CAN_DEBUG_PRINT_HEX(x)    Serial.print(x, HEX)
#else
  #define CAN_DEBUG_PRINT(x)
  #define CAN_DEBUG_PRINTLN(...)
  #define CAN_DEBUG_PRINT_HEX(x)
#endif

extern bool     selfTest;
extern bool     hasNeedleSweep;
extern uint8_t  sweepSpeed;
extern uint16_t maxRPM;
extern uint16_t clusterRPMLimit;
extern uint16_t tempRPM;

extern uint8_t  ecuMode;               // 0 = Bosch ME7.x, 1 = Aftermarket ECU
extern uint32_t aftermarketCanId;      // CAN ID used for aftermarket RPM frame
extern uint8_t  aftermarketByteLow;    // low-byte index (0..7)
extern uint8_t  aftermarketByteHigh;   // high-byte index (0..7)
extern int32_t  aftermarketMultiplierFx; // fixed-point multiplier (value * 1000)
extern int16_t  aftermarketAddition;   // offset added after scaling

extern const int32_t aftermarketMultiplierScale;

extern volatile uint16_t vehicleRPM;
extern volatile uint32_t lastCAN;
extern volatile bool     rpmTruth;
extern volatile bool     canHealthy;

extern bool     hasError;
extern bool     triggerLED;
extern bool     tempNeedleSweep;
extern bool     needleSweepInProgress;
extern bool     tempDiagTest;
extern uint32_t lastMillis;

extern long        frequencyRPM;

extern const ledc_mode_t      rpmLedcMode;
extern const ledc_timer_t     rpmLedcTimer;
extern const ledc_channel_t   rpmLedcChannel;
extern const ledc_timer_bit_t rpmLedcResolution;
extern const uint32_t         rpmLedcMinFreqHz;
extern const uint32_t         rpmLedcDutyOff;
extern const uint32_t         rpmLedcDuty50;

extern TaskHandle_t updateRPMHandle;
extern Preferences    pref;
extern AsyncWebServer server;

// io / main
void setupTimer();
void setFrequencyRPM(long frequencyHz);
void basicInit();
void setupPins();
void needleSweep();
void checkError();

// can
void canInit();
void startCanReceiveTask();
void onBodyRX(const twai_message_t& msg);

// eep
void readEEP();
void writeEEP();

// wifi
void connectWifi();
void checkConnections();
void setupUI();
void generalCallback(const String& key, const String& value);
void extendedCallback(const String& action);
