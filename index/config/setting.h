#include "HardwareSerial.h"
#include "../lib/converters.h"
#include "../lib/defines.h"
#include "../lib/ConfigManager.h"
#include "../MenuDisplay/MenuDisplay.h"
#include "../config/EthernetConfig.h"
#include <WiFi.h>

#ifndef setting_h
#define setting_h


class ConfigureNetwork {
private:
  ConfigManager& config_manager = ConfigManager::getInstance();
  unsigned long previousMillisOne = 0;
  const long intervalOne = 1000000;
  MenuManager menu_manager;
public:
  void set_network_configuration(String operation_mode) {
    switch (operation_mode.toInt()) {
      case 0:
        if (config_manager.get_mac_by_string() == "") {
          M5.lcd.clear();
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println("se configura en una red LAN");
          delay(3000);
        } else {
          str2mac(str, mac);
          LAN = 1;
        }
        break;
      case 1:
        M5.lcd.clear();
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.println("configura una red wifi");
        Serial.println("configura una red wifi");
        delay(5000);
        if (!config_manager.get_wifi_ssid().c_str()) {
          menu_manager.display_menu(2);
        } else {
          menu_manager.display_menu(0);
        }
        conexion(config_manager.get_wifi_ssid().c_str(), config_manager.get_wifi_password().c_str());
        break;
      default:
        M5.lcd.clear();
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.println("configuracion inicial");
        Serial.println("configuracion inicial");
        delay(5000);
        M5.lcd.clear();
        menu_manager.handle_initial_menu();
        break;
    }
  }
  bool conexion(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    M5.Lcd.printf("conectando a: %s\n", ssid);
    Serial.printf("conectando a: %s\n", ssid);
    unsigned long currentMillisOne = millis();
    previousMillisOne = currentMillisOne;
    while (WiFi.status() != WL_CONNECTED) {
      currentMillisOne = millis();
      if (currentMillisOne - previousMillisOne >= intervalOne) {
        return false;
      }
    }
    return true;
  }
};

#endif