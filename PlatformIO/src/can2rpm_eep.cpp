#include "can2rpm_defs.h"

static const char* EEP_NS   = "can2rpm";   // NVS namespace
static const char* EEP_INIT = "initDone";  // first-run sentinel key

void readEEP() {
    DEBUG_PRINTLN("EEPROM: initialising...");
    pref.begin(EEP_NS, false);   // open namespace read/write

    if (!pref.getBool(EEP_INIT, false)) {
        // First run – persist compile-time defaults
        DEBUG_PRINTLN("EEPROM: first run, writing defaults");
        pref.putBool("selfTest",       selfTest);
        pref.putBool("hasNeedleSweep", hasNeedleSweep);
        pref.putUChar("sweepSpeed",    sweepSpeed);
        pref.putUShort("maxRPM",       maxRPM);
        pref.putUShort("clstrRPMLim",  clusterRPMLimit);
        pref.putUShort("tempRPM",      tempRPM);
        pref.putUChar("ecuMode",       ecuMode);
        pref.putUInt("afmCanId",       aftermarketCanId);
        pref.putUChar("afmByteLo",     aftermarketByteLow);
        pref.putUChar("afmByteHi",     aftermarketByteHigh);
        pref.putInt("afmMultFx",       aftermarketMultiplierFx);
        pref.putShort("afmAdd",        aftermarketAddition);
        pref.putBool(EEP_INIT, true);
    } else {
        selfTest        = pref.getBool("selfTest",       false);
        hasNeedleSweep  = pref.getBool("hasNeedleSweep", false);
        sweepSpeed      = pref.getUChar("sweepSpeed",    18);
        maxRPM          = pref.getUShort("maxRPM",       230);
        clusterRPMLimit = pref.getUShort("clstrRPMLim",  7000);
        tempRPM         = pref.getUShort("tempRPM",      0);
        ecuMode         = pref.getUChar("ecuMode",       0);
        aftermarketCanId     = pref.getUInt("afmCanId",  AFTERMARKET_ID);
        aftermarketByteLow   = pref.getUChar("afmByteLo", 2);
        aftermarketByteHigh  = pref.getUChar("afmByteHi", 3);
        aftermarketMultiplierFx = pref.getInt("afmMultFx", 1000);
        aftermarketAddition  = pref.getShort("afmAdd",   0);

        if (aftermarketByteLow > 7)  aftermarketByteLow = 2;
        if (aftermarketByteHigh > 7) aftermarketByteHigh = 3;
        if (ecuMode > 1) ecuMode = 0;
        if (aftermarketMultiplierFx < -1000000) aftermarketMultiplierFx = -1000000;
        if (aftermarketMultiplierFx > 1000000)  aftermarketMultiplierFx = 1000000;
        if (aftermarketAddition < -1000)   aftermarketAddition = -1000;
        if (aftermarketAddition > 1000)    aftermarketAddition = 1000;
    }

    DEBUG_PRINTLN("EEPROM: ready");
}

void writeEEP() {
    DEBUG_PRINTLN("EEPROM: writing...");
    pref.putBool("selfTest",       selfTest);
    pref.putBool("hasNeedleSweep", hasNeedleSweep);
    pref.putUChar("sweepSpeed",    sweepSpeed);
    pref.putUShort("maxRPM",       maxRPM);
    pref.putUShort("clstrRPMLim",  clusterRPMLimit);
    pref.putUShort("tempRPM",      tempRPM);
    pref.putUChar("ecuMode",       ecuMode);
    pref.putUInt("afmCanId",       aftermarketCanId);
    pref.putUChar("afmByteLo",     aftermarketByteLow);
    pref.putUChar("afmByteHi",     aftermarketByteHigh);
    pref.putInt("afmMultFx",       aftermarketMultiplierFx);
    pref.putShort("afmAdd",        aftermarketAddition);
    DEBUG_PRINTLN("EEPROM: written");
}
