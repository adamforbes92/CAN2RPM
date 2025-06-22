void basicInit() {
// basic initialisation - setup pins for IO & setup CAN for receiving...
#if stateDebug
  Serial.begin(baudSerial);
  Serial.println(F("CAN-BUS to Cluster Initialising..."));
  Serial.println(F("Setting up pins..."));
#endif

  setupPins();  // initialise the CAN chip

#if stateDebug
  Serial.println(F("Setup pins complete!"));
#endif

#if stateDebug
  Serial.println(F("CAN Chip Initialising..."));
#endif
  canInit();  // initialise the CAN chip
#if stateDebug
  Serial.println(F("CAN Chip Initialised!"));
#endif
}

void setupPins() {
  // define pin modes for outputs
  pinMode(onboardLED, OUTPUT);  // use the built-in LED for displaying errors!
  pinMode(pinCoil, OUTPUT);
}

void needleSweep() {
  frequencyRPM = 0;
  setFrequencyRPM(frequencyRPM);

  delay(needleSweepDelay);

#if stateDebug
  Serial.println(F("Starting needle sweep..."));
#endif

  while ((frequencyRPM != maxRPM)) {
    setFrequencyRPM(frequencyRPM);

    // scaling?...
    frequencyRPM += stepRPM;
    delay(needleSweepDelay);  // increase or decrease the needle sweep speed in _defs
  }

  while ((frequencyRPM != 0)) {
    setFrequencyRPM(frequencyRPM);

    // scaling?...
    frequencyRPM -= stepRPM;
    delay(needleSweepDelay);  // increase or decrease the needle sweep speed in _defs
  }

  frequencyRPM = 1;
  setFrequencyRPM(frequencyRPM);

  delay(needleSweepDelay);

#if stateDebug
  Serial.println(F("Finished needle sweep!"));
#endif
}

void diagTest() {
  vehicleRPM += 500;

  if (vehicleRPM > clusterRPMLimit) {
    vehicleRPM = 1000;
    frequencyRPM = 1;
  }
  delay(1000);
}