/* 
Basic CAN-BUS converter to coil based output.  Used for MK2 'analog' clusters in ME7.x and aftermarket conversions.
V1.00 - base file

Forbes-Automotive, 2025
*/

// for CAN
#include "can2rpm_defs.h"
#include <ESP32_CAN.h>
ESP32_CAN<RX_SIZE_256, TX_SIZE_16> chassisCAN;

// define two hardware timers for RPM & Speed outputs
hw_timer_t* timer0 = NULL;

bool rpmTrigger = true;
long frequencyRPM = 20;  // 20 to 20000

//if (1) {  // This contains all the timers/Hz/Freq. stuff.  Literally in a //(1) to let Arduino IDE code-wrap all this...
// timer for RPM
void IRAM_ATTR onTimer0() {
  rpmTrigger = !rpmTrigger;
  digitalWrite(pinCoil, rpmTrigger);
}

// setup timers
void setupTimer() {
  timer0 = timerBegin(0, 40, true);  //div 80
  timerAttachInterrupt(timer0, &onTimer0, true);
}

// adjust output frequency
void setFrequencyRPM(long frequencyHz) {
  if (frequencyHz != 0) {
    timerAlarmDisable(timer0);
    timerAlarmWrite(timer0, 1000000l / frequencyHz, true);
    timerAlarmEnable(timer0);
  } else {
    timerAlarmDisable(timer0);
  }
}
//}

void setup() {
  basicInit();   // basic init for setting up IO / CAN
  setupTimer();  // setup the timers (with a base frequency)

  if (hasNeedleSweep) {
    needleSweep();  // carry out needle sweep if defined
  }
}

void loop() {
  // get the easy stuff out the way first
  if (selfTest) {
    diagTest();
  }

  if (hasError) {
    digitalWrite(onboardLED, HIGH);  // light internal LED
  } else {
    digitalWrite(onboardLED, LOW);
  }

  // calculate final frequency:
  frequencyRPM = map(vehicleRPM, 0, clusterRPMLimit, 0, maxRPM);
  setFrequencyRPM(frequencyRPM);  // minimum speed may command 0 and setFreq. will cause crash, so +1 to error 'catch'
}