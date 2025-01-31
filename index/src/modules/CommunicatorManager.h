#include "HardwareSerial.h"
#include <ArduinoJson.h>
#include <M5Stack.h>

#include "esp_wifi.h"
#include <ArduinoJson.h>
#include "NetworkHandler.h"
#include "../../lib/defines.h"
#include "../../lib/ConfigManager.h"
#include "../../lib/writeAndReadEEPROM.h"

#ifndef COMMUNICATOR_MANAGER_H
#define COMMUNICATOR_MANAGER_H

class CommunicatorManager {
private:
  ConfigManager &config_manager = ConfigManager::getInstance();
  NetworkHandler network_handler;
  MemoryHandler memory_handler;
public:
  CommunicatorManager() {
    Serial.printf("status connection net: %d\n", WL_CONNECTED);
    network_handler.init(server_local.c_str(), server_lan.c_str(), api_key);
  }
  void send_registered_communicator(String serial_meter) {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
      network_handler.set_connection_type(true);
    } else {
      Serial.printf("No conectado a Wi-Fi.\n");
      network_handler.set_connection_type(false);
    }
    M5.begin();
    M5.Lcd.begin();
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("enviando registro de controlador...");
    delay(2000);
    String body;
    JsonDocument  doc;
    doc["TypeCommunicatorId"] = 1;
    doc["ProjectId"] = config_manager.get_project_id().toInt();
    doc["name"] = "test2";
    doc["idCommunicator"] = "K001V6";
    doc["state"] = "1";
    doc["model"] = "M5STACK";
    serializeJson(doc, body);
    Serial.println(body);
    delay(1000);
    String rst = network_handler.http_post_req(path_communicator.c_str(), body.c_str(), serial_meter.c_str());
    delay(1000);
    JsonDocument  resp;
    DeserializationError err = deserializeJson(resp, rst);
    if (err) {
      Serial.print("Deserialization failed with code");
      Serial.println(err.f_str());
    } else {
      int communicator_id = resp["body"];
      config_manager.update_field("communicator_id", String(communicator_id));
      memory_handler.set_data(handle, STORAGE, "communicator_id", String(communicator_id).c_str());
      Serial.println(id_communicator);
      delay(10000);
    }
  }
};

#endif