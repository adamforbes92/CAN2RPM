/*
Basic CAN-BUS converter to coil-based RPM output.
Used for MK2 'analog' clusters with ME7.x and aftermarket conversions.
Forbes-Automotive, 2026
*/

#include "can2rpm_defs.h"
#include "can2rpm_vers.h"
#include "can2rpm_tasks.h"
#include "power_manager.h"
#include "wifi_manager.h"

void setup() {
    readEEP(); // read settings from NVS before anything else, as they affect init behaviour

    basicInit(); // Serial, pins, CAN, timer
    setupTimer(); // LEDC timer and channel for RPM output

    if (hasNeedleSweep) {
        needleSweep(); // perform initial needle sweep if enabled (before WiFi so it doesn't block UI)
    }

    connectWifi();
    setupUI();

    // Universal reduced-power module: turns WiFi off 1 min after the last client
    // disconnects, scales CPU 240->80 MHz, releases Bluetooth and kills the
    // onboard LED to cut current draw (and therefore linear-regulator heat).
    power_config_t pcfg = powerDefaultConfig();
    pcfg.verbose = serialDebugWifi;
    powerInit(&pcfg);

    startTasks();   // start all background RTOS tasks
}

void loop() {
    // Needle sweep requested from API
    if (tempNeedleSweep) {
        needleSweep(); 
        tempNeedleSweep = false;
    }
    wifiManagerTick(); // Home WiFi (bridge mode): connection tracking + retry back-off
    vTaskDelay(pdMS_TO_TICKS(100));
}
