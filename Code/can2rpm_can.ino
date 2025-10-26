void canInit() {
  chassisCAN.setRX(pinRX_CAN);
  chassisCAN.setTX(pinTX_CAN);
  chassisCAN.begin();
  chassisCAN.setBaudRate(500000);  // CAN Speed in Hz
  chassisCAN.onReceive(onBodyRX);
}

void onBodyRX(const CAN_message_t& frame) {
#if ChassisCANDebug  // print incoming CAN messages
  Serial.print("Length Recv: ");
  Serial.print(frame.len);
  Serial.print(" CAN ID: ");
  Serial.print(frame.id, HEX);
  Serial.print(" Buffer: ");
  for (uint8_t i = 0; i < frame.len; i++) {
    Serial.print(frame.buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif

  switch (frame.id) {
    case MOTOR1_ID:
      // frame[2] (byte 3) > motor speed low byte
      // frame[3] (byte 4) > motor speed high byte
      // frame[4] (byte 3) > khm speed?
      vehicleRPM = ((frame.buf[3] << 8) | frame.buf[2]) * 0.25;  // conversion: 0.25*HEX
      lastCAN = millis();

      DEBUG_PRINTF("vehicleRPM: ");
      DEBUG_PRINTLN(vehicleRPM);
      break;
    default:
      // do nothing...
      break;
  }
  // do the calc
}