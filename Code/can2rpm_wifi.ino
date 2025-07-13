void setupUI() {
  //Turn off verbose debugging
  ESPUI.setVerbosity(Verbosity::Quiet);
  ESPUI.sliderContinuous = true;

  // create basic tab
  auto tabBasic = ESPUI.addControl(Tab, "", "Setup");
  ESPUI.addControl(Separator, "Needle Sweep", "", Dark, tabBasic);
  bool_NeedleSweep = ESPUI.addControl(Switcher, "Needle Sweep", String(hasNeedleSweep), Dark, tabBasic, generalCallback);
  int16_sweepSpeed = ESPUI.addControl(Slider, "Rate of Change (ms)", String(sweepSpeed), Dark, tabBasic, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_sweepSpeed);
  ESPUI.addControl(Max, "", "50", Dark, int16_sweepSpeed);
  ESPUI.addControl(Button, "Test Needle Sweep", "Test", Dark, tabBasic, extendedCallback, (void *)11);

  ESPUI.addControl(Separator, "Cluster Limits:", "", Dark, tabBasic);
  int16_clusterRPM = ESPUI.addControl(Slider, "Maximum RPM", String(clusterRPMLimit), Dark, tabBasic, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_clusterRPM);
  ESPUI.addControl(Max, "", "9000", Dark, int16_clusterRPM);
  ESPUI.addControl(Button, "Reset", "Reset", Dark, tabBasic, extendedCallback, (void *)12);

  ESPUI.addControl(Separator, "RPM Scaling:", "", Dark, tabBasic);
  int16_RPMScaling = ESPUI.addControl(Slider, "RPM Scaling", String(maxRPM), Dark, tabBasic, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_RPMScaling);
  ESPUI.addControl(Max, "", "500", Dark, int16_RPMScaling);
  ESPUI.addControl(Button, "Reset", "Reset", Dark, tabBasic, extendedCallback, (void *)13);

  ESPUI.addControl(Separator, "RPM Testing:", "", Dark, tabBasic);
  bool_testRPM = ESPUI.addControl(Switcher, "Allow Testing", String(tempDiagTest), Dark, tabBasic, generalCallback);
  int16_tempRPM = ESPUI.addControl(Slider, "Set to RPM", String(tempRPM), Dark, tabBasic, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_tempRPM);
  ESPUI.addControl(Max, "", "9000", Dark, int16_tempRPM);

  ESPUI.addControl(Separator, "Incoming CAN:", "", Dark, tabBasic);
  label_hasCAN = ESPUI.addControl(Label, "Has CAN:", "No", Dark, tabBasic, generalCallback);
  label_RPMCAN = ESPUI.addControl(Label, "RPM:", "0", Dark, tabBasic, generalCallback);

  //Finally, start up the UI.
  //This should only be called once we are connected to WiFi.
  ESPUI.begin(wifiHostName);
}

void updateCallback(Control *sender, int type) {
  updates = (sender->value.toInt() > 0);
}

void getTimeCallback(Control *sender, int type) {
  if (type == B_UP) {
    ESPUI.updateTime(mainTime);
  }
}

void graphAddCallback(Control *sender, int type) {
  if (type == B_UP) {
    ESPUI.addGraphPoint(graph, random(1, 50));
  }
}

void graphClearCallback(Control *sender, int type) {
  if (type == B_UP) {
    ESPUI.clearGraph(graph);
  }
}

void generalCallback(Control *sender, int type) {
#ifdef serialDebugWifi
  Serial.print("CB: id(");
  Serial.print(sender->id);
  Serial.print(") Type(");
  Serial.print(type);
  Serial.print(") '");
  Serial.print(sender->label);
  Serial.print("' = ");
  Serial.println(sender->value);
#endif

  uint8_t tempID = int(sender->id);
  switch (tempID) {
    case 3:
      hasNeedleSweep = sender->value.toInt();
      break;
    case 4:
      sweepSpeed = sender->value.toInt();
      break;
    case 9:
      clusterRPMLimit = sender->value.toInt();
      break;
    case 14:
      maxRPM = sender->value.toInt();
      break;
    case 19:
      tempDiagTest = sender->value.toInt();
      break;
    case 20:
      tempRPM = sender->value.toInt();
      break;
  }
}

void extendedCallback(Control *sender, int type, void *param) {
#ifdef serialDebugWifi
  Serial.print("CB: id(");
  Serial.print(sender->id);
  Serial.print(") Type(");
  Serial.print(type);
  Serial.print(") '");
  Serial.print(sender->label);
  Serial.print("' = ");
  Serial.println(sender->value);
  Serial.print("param = ");
  Serial.println((long)param);
#endif

  uint8_t tempID = int(sender->id);
  switch (tempID) {
    case 7:
      if (type == B_UP) {
        tempNeedleSweep = true;
      }
      break;
    case 12:
      if (type == B_UP) {
        clusterRPMLimit = 7000;
        ESPUI.updateSlider(int16_clusterRPM, clusterRPMLimit);
      }
      break;
    case 17:
      if (type == B_UP) {
        maxRPM = 230;
        ESPUI.updateSlider(int16_RPMScaling, maxRPM);
      }
      break;
  }
}

void connectWifi() {
  int connect_timeout;

  WiFi.setHostname(wifiHostName);
  DEBUG_PRINTLN("Begin WiFi...");
  DEBUG_PRINTLN("Creating access point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(wifiHostName);

  connect_timeout = 20;
  do {
    delay(250);
    DEBUG_PRINTF(".");
    connect_timeout--;
  } while (connect_timeout);
}

void disconnectWifi() {
  DEBUG_PRINTF("Number of connections: ");
  DEBUG_PRINTLN(WiFi.softAPgetStationNum());

  if (WiFi.softAPgetStationNum() == 0) {
    DEBUG_PRINTLN("No connections, turning off");
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);

    tempDiagTest = false;
  }
}

void checkConnections() {
  DEBUG_PRINTF("Number of connections: ");
  DEBUG_PRINTLN(WiFi.softAPgetStationNum());

  if (WiFi.softAPgetStationNum() > 0) {
    hasError = true;
  }
}

void textCallback(Control *sender, int type) {
  //This callback is needed to handle the changed values, even though it doesn't do anything itself.
}

void randomString(char *buf, int len) {
  for (auto i = 0; i < len - 1; i++)
    buf[i] = random(0, 26) + 'A';
  buf[len - 1] = '\0';
}