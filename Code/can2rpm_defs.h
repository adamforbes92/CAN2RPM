/* Defines */
// Debug statements
#define ChassisCANDebug 0  // if 1, will print CAN 2 (Chassis) messages ** CAN CHANGE THIS **
#define stateDebug 1       // if 1, will use Serial talkback ** CAN CHANGE THIS **
#define selfTest 1        // increase RPM/speed slowly, flash lights.  For debug only, disable on release! ** CAN CHANGE THIS **

// setup - main inputs
#define hasNeedleSweep 0  // do needle sweep on power up? ** CAN CHANGE THIS **
#define needleSweepDelay 60

// setup - Hz adjustment
#define maxRPM 230    // max RPM in Hz for the cluster (for needle sweep) ** CAN CHANGE THIS **

// setup - RPM & speed limits
#define clusterRPMLimit 7000   // rpm ** CAN CHANGE THIS **

// setup - step changes (for needle sweep)
#define stepRPM 1

// setup - pins (output)
#define pinRX_CAN 16  // pin output for SN65HVD230 (CAN_RX)
#define pinTX_CAN 17  // pin output for SN65HVD230 (CAN_TX)
#define pinCoil 25    // pin output for RPM (MK2/High Output Coil Trigger)
#define onboardLED 2  // pin onboard LED

// Baud Rates
#define baudSerial 115200  // baud rate for debug

extern uint16_t vehicleRPM = 1;      // current RPM.  If no CAN, this will catch dividing by zero by the map function

// onboard LED for error
extern bool hasError = false;

// define CAN Addresses.  All not req. but here for keepsakes
#define MOTOR1_ID 0x280

extern void basicInit(void);
extern void canInit(void);
extern void onBodyRX(void);
extern void needleSweep(void);
extern void setupPins(void);
extern void blinkLED(int duration, int flashes, bool boolEPC, bool boolEML, bool boolRPM, bool boolSpeed);