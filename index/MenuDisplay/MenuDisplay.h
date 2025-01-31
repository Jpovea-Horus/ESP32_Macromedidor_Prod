#include "WString.h"
#include "esp32-hal.h"
#include "HardwareSerial.h"
#include "../lib/ConfigManager.h"
#include "../lib/writeAndReadEEPROM.h"
#include "../config/EthernetConfig.h"

#ifndef MENUDISPLAY_H
#define MENUDISPLAY_H

MemoryHandler memory_handler;

class MenuManager {
private:
  ConfigManager& config = ConfigManager::getInstance();
  String key;
  String addresses[MAX_METERS];
  int initial_data = 0;
  int get_value_from_serial() {
    int x = 0;
    String inString = "";

    while (true) {
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        if (isDigit(inChar)) {
          inString += inChar;
        }
      }

      if (!inString.isEmpty()) {
        x = inString.toInt();
        break;
      }
    }

    return x;
  }
  String get_input_string() {
    String inString = "";
    while (Serial.available() > 0) {
      Serial.read();
    }

    while (true) {
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        inString += inChar;
      }
      if (!inString.isEmpty()) {
        break;
      }
    }
    return inString;
  }
  void initialize_settings(const char* key, const char* value) {
    memory_handler.set_data(handle, STORAGE, key, value);
  }
public:
  void display_menu(int item) {
    switch (item) {
      case 0:
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.print("Ingrese codigo del proyecto");
        Serial.print("#Ingrese codigo del proyecto: ");
        config.update_field("project_id", get_input_string());
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.println();
        Serial.println(config.get_project_id());
        initialize_settings("project_id", config.get_project_id().c_str());
        delay(5000);
        display_menu(1);
        break;
      case 1:
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.println("Ingrese la cantidad de medidores: ");
        Serial.println("#Ingrese la cantidad de medidores: ");
        config.update_field("num_meters_to_install", get_input_string().c_str());
        initialize_settings("num_meters", config.get_num_meters_to_install().c_str());
        for (int i = 1; i <= config.get_num_meters_to_install().toInt(); i++) {
          String serial_meter = "serial_" + String(i);
          String address = "0" + String(i);
          Serial.printf("registro de variables: %s\n", serial_meter);
          initialize_settings(serial_meter.c_str(), "0");
          addresses[i] = address;
        }
        config.set_meter_addresses(addresses, config.get_num_meters_to_install().toInt());
        config.update_field("status", "1");
        initialize_settings("status", "1");
        break;
      case 2:
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.println("##SSID de tu red: ");
        Serial.print("##SSID de tu red: ");
        config.update_field("wifi_ssid", get_input_string().c_str());
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.println(config.get_wifi_ssid());
        Serial.println(config.get_wifi_ssid());
        initialize_settings("wifi_ssid", config.get_wifi_ssid().c_str());
        if (config.get_wifi_password() == "404") {
          M5.Lcd.setTextColor(WHITE);
          M5.Lcd.print("##password: ");
          Serial.print("##password: ");
          config.update_field("wifi_pass", get_input_string());
          M5.Lcd.setTextColor(GREEN);
          M5.Lcd.println("*************");
          Serial.println("*************");
          initialize_settings("wifi_pass", config.get_wifi_password().c_str());
        }
        delay(5000);
        display_menu(0);
        break;
    }
  }
  void handle_initial_menu() {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("1. Red WiFi");
    Serial.println("1. Red WiFi");
    M5.Lcd.drawCircle(155, 7, 7, WHITE);
    M5.Lcd.println("2. Red LAN");
    Serial.println("2. Red LAN");
    M5.Lcd.drawCircle(155, 26, 7, WHITE);
    M5.Lcd.drawXBitmap(axis_x, axis_y, logo, logo_width, logo_height, TFT_WHITE, TFT_BLACK);
    initial_data = get_value_from_serial();
    switch (initial_data) {
      case 1:
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.println("1. Red WiFi: ");
        Serial.println("1. Red WiFi: #");
        M5.Lcd.drawCircle(155, 7, 7, WHITE);
        M5.Lcd.fillCircle(155, 7, 6, GREEN);
        M5.Lcd.println("2. Red LAN");
        Serial.println("2. Red LAN: ");
        config.update_field("operation_mode", "1");
        initialize_settings("operation_mode", config.get_operation_mode().c_str());
        display_menu(2);
        delay(3000);
        break;
      case 2:
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.println("1. Red WiFi: ");
        Serial.println("1. Red WiFi: ");
        M5.Lcd.println("2. Red LAN");
        Serial.println("2. Red LAN: #");
        M5.Lcd.drawCircle(155, 26, 7, WHITE);
        M5.Lcd.fillCircle(155, 26, 6, GREEN);
        str2mac(str, mac);
        key = str;
        Serial.println(str);
        delay(1000);
        LAN = 1;
        config.update_field("operation_mode", "0");
        initialize_settings("operation_mode", config.get_operation_mode().c_str());
        display_menu(0);
        break;
    }
  }
};

#endif