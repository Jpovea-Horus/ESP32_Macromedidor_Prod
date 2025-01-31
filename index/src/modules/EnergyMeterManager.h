#include <ArduinoJson.h>
#include <M5Stack.h>
#include "../../lib/defines.h"
#include "../../lib/ConfigManager.h"
#include "NetworkHandler.h"

#ifndef ENERGY_METER_MANAGER_H
#define ENERGY_METER_MANAGER_H


class EnergyMeterManager {
private:
  ConfigManager &config_manager = ConfigManager::getInstance();
  NetworkHandler network_handler;

public:

  EnergyMeterManager() {
    Serial.printf("status connection net: %d\n", WL_CONNECTED);
    network_handler.init(server_local.c_str(), server_lan.c_str(), api_key);
  }

  String send_regiterd_meter(String serial_meter) {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
      network_handler.set_connection_type(true);
    } else {
      Serial.printf("No conectado a Wi-Fi.\n");
      network_handler.set_connection_type(false);
    }
    M5.Lcd.printf("Enviando registro del medidor: %s\n", serial_meter);
    delay(2000);
    String body;
    JsonDocument  doc;
    // en produccion son: CommunicatorId, relationTc
    doc["CommunicatorId"] = config_manager.get_communicator_id();
    doc["serial"] = serial_meter;
    doc["model"] = "SMD630MCT";
    doc["relationTc"] = "200/5";
    doc["maximumCurrent"] = "5000";
    doc["isNewMeter"] = true;
    doc["enabled"] = true;
    doc["powerFactor"] = 40;

    JsonObject variables = doc["variables"].to<JsonObject>();
    variables["VFA"] = 120;
    variables["VFB"] = 122;
    variables["VFC"] = 125;

    JsonObject variablesFactor = doc["variablesFactor"].to<JsonObject>();
    variablesFactor["VFA"] = 120;
    variablesFactor["VFB"] = 122;
    variablesFactor["VFC"] = 125;

    serializeJson(doc, body);
    delay(1000);
    String rst = network_handler.http_post_req(path_registered.c_str(), body.c_str(), serial_meter.c_str());
    delay(1000);
    JsonDocument resp;
    DeserializationError err = deserializeJson(resp, rst);
    if (err) {
      Serial.print("Deserialization failed with code");
      Serial.println(err.f_str());
      return "404";
    }
    String status_code = resp["status"];
    return status_code;
  }
};

#endif