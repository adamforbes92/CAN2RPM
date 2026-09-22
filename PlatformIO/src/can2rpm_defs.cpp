#include "can2rpm_defs.h"

// ─────────────────────────────────────────────────────────────────
// Global variable definitions
// ─────────────────────────────────────────────────────────────────
bool     selfTest        = false;   // self-test mode
bool     hasNeedleSweep  = false;   // perform needle sweep on boot
uint8_t  sweepSpeed      = 18;      // needle sweep step delay (ms)
uint16_t maxRPM          = 230;     // maximum output frequency (Hz)
uint16_t clusterRPMLimit = 7000;    // vehicle RPM that maps to maxRPM
uint16_t tempRPM         = 3000;    // fixed RPM used in diag test mode

uint8_t  ecuMode              = 0;          // default: Bosch ME7.x
uint32_t aftermarketCanId     = AFTERMARKET_ID;
uint8_t  aftermarketByteLow   = 2; // low byte
uint8_t  aftermarketByteHigh  = 3; // high byte
int32_t  aftermarketMultiplierFx = 1000; // fixed-point multiplier; 1000 == 1.000
int16_t  aftermarketAddition  = 0; // offset added after scaling (after byte extraction)

const int32_t aftermarketMultiplierScale = 1000;

volatile uint16_t vehicleRPM = 0;   // current engine RPM from CAN
volatile uint32_t lastCAN    = 0;   // millis() timestamp of last CAN frame
volatile bool     rpmTruth   = false; // true when decoded RPM is in accepted range
volatile bool     canHealthy = false; // true when CAN frames are being received within timeout

bool     hasError        = false;   // CAN timeout / error state
bool     triggerLED      = false;   // LED blink toggle state
bool     tempNeedleSweep = false;   // flag: trigger needle sweep from web UI
bool     needleSweepInProgress = false; // flag: prevent overlapping sweeps
bool     tempDiagTest    = false;   // flag: use fixed tempRPM instead of CAN RPM
uint32_t lastMillis      = 0;       // RPM output rate-limiter timestamp

long        frequencyRPM = 0;      // current output frequency (Hz)

const ledc_mode_t      rpmLedcMode       = LEDC_LOW_SPEED_MODE;
const ledc_timer_t     rpmLedcTimer      = LEDC_TIMER_0;
const ledc_channel_t   rpmLedcChannel    = LEDC_CHANNEL_0;
const ledc_timer_bit_t rpmLedcResolution = LEDC_TIMER_10_BIT;
const uint32_t         rpmLedcMinFreqHz  = 10;
const uint32_t         rpmLedcDutyOff    = 0;
const uint32_t         rpmLedcDuty50     = 512;

TaskHandle_t updateRPMHandle = NULL;

Preferences    pref;
AsyncWebServer server(80);
