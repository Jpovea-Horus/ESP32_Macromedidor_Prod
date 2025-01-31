bool queryDBSerial(String serial) {
  bool statusMeter = false;
  int  apiResp = 0;
  M5.lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("pregunta si el serial esta en DB");
  M5.lcd.clear();
  delay(2000);
  String statusSerial = httpGETReq(server, serverLan, statusSerialPath, serial);
  Serial.println(statusSerial);
  StaticJsonDocument<300> resp;
  DeserializationError err = deserializeJson(resp, statusSerial);
  if (err) {
    M5.Lcd.print("Deserialization failed with code");
    M5.Lcd.println(err.f_str());
  } else {
    apiResp = resp["body"];
  }
  M5.Lcd.println(apiResp);
  delay(3000);
  if (apiResp != 0) {
    return statusMeter = true;
  } else {
    return statusMeter = false;
  }
}
