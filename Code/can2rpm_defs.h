#include <ESP32_CAN.h>
#include <Arduino.h>
#include "TickTwo.h"      // for repeated tasks
#include <Preferences.h>  // for eeprom/remember settings
#include <ESPUI.h>
#include <WiFi.h>
#include <ESPmDNS.h>

/* Defines */
// Debug statements
#define ChassisCANDebug 0       // if 1, will print CAN 2 (Chassis) messages ** CAN CHANGE THIS **
#define serialDebug 1           // if 1, will use Serial talkback ** CAN CHANGE THIS **
#define serialDebugWifi 1       // if 1, will use Serial talkback for WiFi
#define wifiHostName "CAN2RPM"  // the WiFi name
#define eepRefresh 2000         // EEPROM Refresh in ms
#define wifiDisable 30000       // turn off WiFi in ms

// setup - main inputs
extern bool selfTest = false;  // for testing only, vary final pwmFrequency for speed ** CAN CHANGE THIS **

// setup - needle sweep
extern bool hasNeedleSweep = false;  // for needle sweep ** CAN CHANGE THIS **
extern uint8_t sweepSpeed = 18;      // for needle sweep rate of change (in ms) ** CAN CHANGE THIS **

// setup - Hz adjustment
#define rpmDelay 80                      // update rate from CAN to RPM.  Reduces the loop work
extern uint16_t maxRPM = 230;            //max RPM in Hz for the cluster (for needle sweep) ** CAN CHANGE THIS **
extern uint16_t clusterRPMLimit = 7000;  // min frequency for top speed using the 02J / 02M hall sensor  ** CAN CHANGE THIS **
extern uint16_t tempRPM = 3000;          // min frequency for top speed using the 02J / 02M hall sensor  ** CAN CHANGE THIS **

// setup - pins (output)
#define pinRX_CAN 17  // pin output for SN65HVD230 (CAN_RX)
#define pinTX_CAN 16  // pin output for SN65HVD230 (CAN_TX)
#define pinCoil 25    // pin output for RPM (MK2/High Output Coil Trigger)
#define onboardLED 2  // pin onboard LED

// Baud Rates
#define baudSerial 115200  // baud rate for debug

extern uint16_t vehicleRPM = 1;  // current RPM.  If no CAN, this will catch dividing by zero by the map function

// errors & temporary states
extern bool hasError = false;         // has error, for LED
extern bool triggerLED = false;
extern bool tempNeedleSweep = false;  // for activating needle sweep in wifi
extern bool tempDiagTest = false;     // for diag testing

extern uint32_t lastMillis = 0;  // Counter for sending frames x ms
extern uint32_t lastCAN = 0;     // last CAN message

// define CAN Address to filter on.  Only looking for RPM, so no need to clog
#define MOTOR1_ID 0x280

extern void basicInit(void);
extern void canInit(void);
extern void onBodyRX(void);
extern void needleSweep(void);
extern void setupPins(void);
extern void blinkLED(int duration, int flashes, bool boolEPC, bool boolEML, bool boolRPM, bool boolSpeed);
extern void checkError();

//Function Prototypes
extern void connectWifi();
extern void disconnectWifi();
extern void checkConnections();
extern void setupUI();
extern void textCallback(Control *sender, int type);
extern void generalCallback(Control *sender, int type);
extern void updateCallback(Control *sender, int type);
extern void getTimeCallback(Control *sender, int type);
extern void graphAddCallback(Control *sender, int type);
extern void graphClearCallback(Control *sender, int type);
extern void randomString(char *buf, int len);
extern void extendedCallback(Control *sender, int type, void *param);

// EEPROM / Preferences
extern void readEEP();
extern void writeEEP();

// UI handles
uint16_t bool_NeedleSweep, int16_sweepSpeed;
uint16_t int16_clusterRPM, int16_RPMScaling, int16_tempRPM, bool_testRPM;

uint16_t bool_positiveOffset, int16_speedOffset;
int label_hasCAN, label_RPMCAN;

uint16_t graph;
uint16_t mainTime;
volatile bool updates = false;

// if serialDebug is on, allow Serial talkback
#ifdef serialDebug
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(x...) Serial.printf(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x...)
#endif