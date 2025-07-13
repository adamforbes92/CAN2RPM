void readEEP() {
#if serialDebug
  DEBUG_PRINTLN("EEPROM initialising!");
#endif
  // use ESP32's 'Preferences' to remember settings.  Begin by opening the various types.  Use 'false' for read/write.  True just gives read access
  pref.begin("selfTest", false);
  pref.begin("hasNeedleSweep", false);
  pref.begin("sweepSpeed", false);
  pref.begin("maxRPM", false);
  pref.begin("clusterRPMLimit", false);
  pref.begin("tempRPM", false);

  // first run comes with EEP valve of 255, so write actual values.  If found/match SW version, read all the values
  if (pref.getUChar("selfTest") == 255) {
#if serialDebug
    DEBUG_PRINTLN("First run, set Bluetooth module, write Software Version etc");
    DEBUG_PRINTLN(pref.getUChar("testSpeedo"));
#endif
    pref.putBool("selfTest", selfTest);
    pref.putBool("hasNeedleSweep", hasNeedleSweep);
    pref.putUChar("sweepSpeed", sweepSpeed);
    pref.putUShort("maxRPM", maxRPM);
    pref.putUShort("clusterRPMLimit", clusterRPMLimit);
    pref.putUShort("tempRPM", tempRPM);
  } else {
    selfTest = pref.getBool("selfTest", false);
    hasNeedleSweep = pref.getBool("hasNeedleSweep", false);
    sweepSpeed = pref.getUChar("sweepSpeed", 18);
    maxRPM = pref.getUShort("maxRPM", 230);
    clusterRPMLimit = pref.getUShort("clusterRPMLimit", 7000);
    tempRPM = pref.getUShort("tempRPM", 0);
  }
#if serialDebug
  DEBUG_PRINTLN("EEPROM initialised with...");
#endif
}

void writeEEP() {
#if serialDebug
  DEBUG_PRINTLN("Writing EEPROM...");
#endif

  // update EEP only if changes have been made
  pref.putBool("selfTest", selfTest);
  pref.putBool("hasNeedleSweep", hasNeedleSweep);
  pref.putUChar("sweepSpeed", sweepSpeed);
  pref.putUShort("maxRPM", maxRPM);
  pref.putUShort("clusterRPMLimit", clusterRPMLimit);
  pref.putUShort("tempRPM", tempRPM);

#if serialDebug
  DEBUG_PRINTLN("Written EEPROM with data:...");
  DEBUG_PRINTLN(selfTest);
  DEBUG_PRINTLN(hasNeedleSweep);
  DEBUG_PRINTLN(sweepSpeed);
  DEBUG_PRINTLN(maxRPM);
  DEBUG_PRINTLN(clusterRPMLimit);
  DEBUG_PRINTLN(tempRPM);
#endif
}
