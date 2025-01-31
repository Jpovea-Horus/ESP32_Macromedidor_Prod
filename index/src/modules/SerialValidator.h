#include <M5Stack.h>
#include "HardwareSerial.h"
#include "esp_wifi.h"
#include <ArduinoJson.h>
#include "NetworkHandler.h"
#include "../../lib/defines.h"

#ifndef SERIAL_VALIDATOR_H
#define SERIAL_VALIDATOR_H



class SerialDBQuery {
private:
  EthernetConfig& ethernet_config = EthernetConfig::getInstance();
  NetworkHandler network_handler;
  bool status_meter = false;
  int api_res = 0;
  String status_serial;

public:
  SerialDBQuery() {
    Serial.printf("status connection net: %d\n", WL_CONNECTED);
    if (!WL_CONNECTED) {
      network_handler.init(server_local.c_str(), server_lan.c_str(), api_key);
    }
  }
  bool check_serial_in_db(String& serial_meter) {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
      Serial.printf("Wi-Fi conectado al SSID: %s\n", ap_info.ssid);
      network_handler.set_connection_type(true);
      status_serial = network_handler.http_get_req(status_serial_path.c_str(), serial_meter.c_str());

    } else {
      Serial.printf("No conectado a Wi-Fi.\n");
      network_handler.set_connection_type(false);
      status_serial = ethernet_config.http_get_req(
        server_lan.c_str(),
        status_serial_path.c_str(),
        serial_meter.c_str());
    }
    M5.Lcd.clear();
    M5.Lcd.println("check if the meter exists");
    delay(2000);


    JsonDocument resp;
    DeserializationError error = deserializeJson(resp, status_serial);

    if (error) {
      M5.Lcd.print("Failed deserialization with code");
      M5.Lcd.println(error.f_str());
    } else {
      api_res = resp["body"];
    }
    delay(3000);
    if (api_res != 0) {
      return status_meter = true;
    } else {
      return status_meter;
    }
  }
};

#endif