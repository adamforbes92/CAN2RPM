/*
Basic CAN-BUS converter to coil-based RPM output.
Used for MK2 'analog' clusters with ME7.x and aftermarket conversions.
Forbes-Automotive, 2026
*/

#include "can2rpm_defs.h"
#include "can2rpm_vers.h"
#include "can2rpm_tasks.h"

void setup() {
    readEEP(); // read settings from NVS before anything else, as they affect init behaviour

    basicInit(); // Serial, pins, CAN, timer
    setupTimer(); // LEDC timer and channel for RPM output

    if (hasNeedleSweep) {
        needleSweep(); // perform initial needle sweep if enabled (before WiFi so it doesn't block UI)
    }

    connectWifi();
    setupUI();

    startTasks();   // start all background RTOS tasks
}

void loop() {
    // Needle sweep requested from API
    if (tempNeedleSweep) {
        needleSweep(); 
        tempNeedleSweep = false;
    }
}
