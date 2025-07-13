/* 
Basic CAN-BUS converter to coil based output.  Used for MK2 'analog' clusters in ME7.x and aftermarket conversions.
V1.00 - base file
V1.01 - added error LED for no CAN messages
V1.02 - added WiFi Setup pages

Forbes-Automotive, 2025
*/

// for CAN
#include "can2rpm_defs.h"
ESP32_CAN<RX_SIZE_256, TX_SIZE_16> chassisCAN;             // CAN Lib
TickTwo tickEEP(writeEEP, eepRefresh);                     // timer for EEPROM saving
TickTwo tickError(checkError, 500);                        // timer for error checking
TickTwo tickWiFi(disconnectWifi, wifiDisable);             // timer for disconnecting wifi after 30s if no connections - saves power
TickTwo tickWiFiConn(checkConnections, wifiDisable / 30);  // timer for checking wifi connections.  If > 0, flash the onboard LED
Preferences pref;                                          // for EEPROM / saving preferences

// define one hardware timers for RPM output
hw_timer_t* timer0 = NULL;

bool rpmTrigger = true;
long frequencyRPM = 20;  // 20 to 20000

// timer for RPM
void IRAM_ATTR onTimer0() {
  rpmTrigger = !rpmTrigger;
  digitalWrite(pinCoil, rpmTrigger);  // flip-flop RPM trigger to create pulse
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
    timerAlarmDisable(timer0);  // error catch, if freq. set to zero
  }
}

void setup() {
  readEEP();             // read the EEPROM for previous states
  tickEEP.start();       // begin ticker for the EEPROM
  tickError.start();
  tickWiFi.start();      // begin ticker for the WiFi (to turn off after 60s)
  tickWiFiConn.start();  // begin ticker to check the number of WiFi connections (to flash the onboard LED)

  basicInit();   // basic init for setting up IO / CAN
  setupTimer();  // setup the timers (with a base frequency)

  if (hasNeedleSweep) {
    needleSweep();  // carry out needle sweep if enabled
  }

  connectWifi();         // enable / start WiFi
  WiFi.setSleep(false);  // for the ESP32: turn off sleeping to increase UI responsivness (at the cost of power use)
  setupUI();             // setup wifi user interface
}

void loop() {
  // get the easy stuff out the way first
  tickEEP.update();       // refresh the EEP ticker
  tickError.update();
  tickWiFi.update();      // refresh the WiFi ticker
  tickWiFiConn.update();  // refresh the WiFi connection ticker

  if (tempNeedleSweep) {
    // can't do this in the WiFi loop - so set flag in there and process here
    needleSweep();
    tempNeedleSweep = false;  // reset to false to stop it continuing to run
  }

  // if last CAN message was >500ms ago, it's in an error state, set flag
  if ((millis() - lastCAN) > 500) {
    hasError = true;
    ESPUI.updateLabel(label_hasCAN, "No");
    ESPUI.updateLabel(label_RPMCAN, "CAN RPM: 0");
  } else {
    hasError = false;
    ESPUI.updateLabel(label_hasCAN, "Yes");
    char buf[32];
    sprintf(buf, "CAN RPM: %d", vehicleRPM);
    ESPUI.updateLabel(label_RPMCAN, String(buf));
  }

  if ((millis() - lastMillis) > rpmDelay) {  // check to see if x ms (rpmDelay) has elapsed - slow down the RPM changes!
    lastMillis = millis();

    // diagTest is in 'WiFi' and will allow the user to set a fixed RPM for calibration
    if (tempDiagTest) {
      frequencyRPM = map(tempRPM, 0, clusterRPMLimit, 0, maxRPM);
      setFrequencyRPM(frequencyRPM);  // minimum speed may command 0 and setFreq. will cause crash, so +1 to error 'catch'
    } else {
      // calculate final frequency for RPM:
      frequencyRPM = map(vehicleRPM, 0, clusterRPMLimit, 0, maxRPM);
      setFrequencyRPM(frequencyRPM);  // minimum speed may command 0 and setFreq. will cause crash, so +1 to error 'catch'
    }
  }
}