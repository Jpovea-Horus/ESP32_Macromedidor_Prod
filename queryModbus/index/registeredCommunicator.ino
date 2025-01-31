#include "lib/defines.h"
#include "network/packagesent.h"
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

void sendRegiterdCommunicator(String serial) {
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.println("enviando registro de controlador...");
  delay(2000);
  String body;
  StaticJsonDocument<300> doc;
  // en produccion es TypeCommunicatorId, ProjectId
  doc["TypeCommunicatorId"] = 1;
  doc["ProjectId"] = projectId;
  doc["name"] = "test2";
  doc["idCommunicator"] = "K001V6";
  doc["state"] = "1";
  doc["model"] = "M5STACK";
  serializeJson(doc, body);
  delay(1000);
  String rst = httpPOSTReq(server, serverLan, pathCommunicator, body, serial);
  Serial.println(rst);
  delay(1000);
  StaticJsonDocument<300> resp;
  DeserializationError err = deserializeJson(resp, rst);
  if (err) {
    Serial.print("Deserialization failed with code");
    Serial.println(err.f_str());
  } else {
    idCommunicator = resp["body"];
    delay(10000);
    writeStringToEEPROM(123, String(idCommunicator));
  }
}

String sendRegiterdMeter(String serial) {

  M5.Lcd.println("enviando registro del medidor: ");
  M5.Lcd.print(serial);
  delay(2000);
  String body;
  StaticJsonDocument<300> doc;
  // en produccion son: CommunicatorId, relationTc
  doc["CommunicatorId"] = idCommunicator;
  doc["serial"] = serial;
  doc["model"] = "SMD630MCT";
  doc["relationTc"] = "200/5";
  doc["maximumCurrent"] = "5000";
  doc["isNewMeter"] = true;
  doc["enabled"] = true;
  doc["powerFactor"] = 40;

  JsonObject variables = doc.createNestedObject("variables");
  variables["VFA"] = 120;
  variables["VFB"] = 122;
  variables["VFC"] = 125;

  JsonObject variablesFactor = doc.createNestedObject("variablesFactor");
  variablesFactor["VFA"] = 120;
  variablesFactor["VFB"] = 122;
  variablesFactor["VFC"] = 125;

  serializeJson(doc, body);
  delay(1000);
  Serial.print("serial: ");
  Serial.println(serial);
  String rst = httpPOSTReq(server, serverLan, pathRegistered, body, serial);
  M5.Lcd.println(rst);
  M5.Lcd.println(body);
  return rst;
}