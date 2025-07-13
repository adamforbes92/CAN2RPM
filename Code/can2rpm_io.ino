void basicInit() {
// basic initialisation - setup pins for IO & setup CAN for receiving...
#if serialDebug
  Serial.begin(baudSerial);
  delay(200);  // allow Serial to start
  DEBUG_PRINTLN("CAN-BUS to RPM Initialising...");
#endif

  DEBUG_PRINTLN("Setting up pins...");
  setupPins();  // initialise the CAN chip
  DEBUG_PRINTLN("Setup pins complete!");

  DEBUG_PRINTLN("CAN Chip Initialising...");
  canInit();  // initialise the CAN chip
  DEBUG_PRINTLN("CAN Chip Initialised!");
}

void setupPins() {
  // define pin modes for outputs
  pinMode(onboardLED, OUTPUT);  // use the built-in LED for displaying errors!
  pinMode(pinCoil, OUTPUT);
}

void needleSweep() {
  DEBUG_PRINTLN("Starting needle sweep...");
  frequencyRPM = 0;
  setFrequencyRPM(frequencyRPM);
  delay(sweepSpeed);

  while ((frequencyRPM != maxRPM)) {
    setFrequencyRPM(frequencyRPM);

    frequencyRPM += 1;
    delay(sweepSpeed);  // increase or decrease the needle sweep speed in _defs
  }
  // pause at top
  delay(sweepSpeed * 50);

  while ((frequencyRPM != 0)) {
    setFrequencyRPM(frequencyRPM);

    // scaling?...
    frequencyRPM -= 1;
    delay(sweepSpeed);  // increase or decrease the needle sweep speed in _defs
  }

  frequencyRPM = 0;
  setFrequencyRPM(frequencyRPM);
  delay(sweepSpeed);

  DEBUG_PRINTLN("Finished needle sweep!");
}

// not used but kept for testing
void diagTest() {
  vehicleRPM += 500;

  if (vehicleRPM > clusterRPMLimit) {
    vehicleRPM = 1000;
    frequencyRPM = 1;
  }
  delay(1000);
}

void checkError() {
  if (hasError) {
    triggerLED = !triggerLED;
  }else{
    triggerLED = false;
  }

  if (triggerLED) {
    digitalWrite(onboardLED, HIGH);  // turn internal LED on
  } else {
    digitalWrite(onboardLED, LOW);  // turn internal LED off
  }
}