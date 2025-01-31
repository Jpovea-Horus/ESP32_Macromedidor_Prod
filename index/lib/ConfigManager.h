#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include "defines.h"

class ConfigManager {
private:
  static ConfigManager* instance;
  meter_communicator_config configuration;

  ConfigManager() {
    configuration = meter_communicator_config();
  }

public:
  static ConfigManager& getInstance() {
    if (instance == nullptr) {
      instance = new ConfigManager();
    }
    return *instance;
  }

  void update_field(const String& field_name, String value) {
    if (field_name == "project_id") {
      configuration.project_id = value;
    } else if (field_name == "communicator_id") {
      configuration.communicator_id = value;
    } else if (field_name == "operation_mode") {
      configuration.operation_mode = value;
    } else if (field_name == "registered") {
      configuration.registered = value;
    } else if (field_name == "wifi_ssid") {
      configuration.network_ssid = value;
    } else if (field_name == "wifi_pass") {
      configuration.network_password = value;
    } else if (field_name == "mac_by_string") {
      configuration.mac_by_string = value;
    } else if (field_name == "num_meters_to_install") {
      configuration.num_meters_to_install = value;
    } else if (field_name == "status") {
      configuration.status = value.toInt();
    } else {
      Serial.println("Campo no válido: " + field_name);
    }
  }

  void set_meter_serials(const String serials[], int count) {
    if (count <= MAX_METERS) {
      for (int i = 0; i < count; i++) {
        configuration.meter_serials[i] = serials[i];
      }
    } else {
      Serial.println("Demasiados medidores");
    }
  }

  void set_meter_addresses(const String addresses[], int count) {
    if (count <= MAX_METERS) {
      for (int i = 0; i < count; i++) {
        configuration.meter_addresses[i] = addresses[i];
      }
    } else {
      Serial.println("Demasiadas direcciones de medidores");
    }
  }

  String get_project_id() const {
    return configuration.project_id;
  }

  String get_communicator_id() const {
    return configuration.communicator_id;
  }

  String get_operation_mode() const {
    return configuration.operation_mode;
  }

  String get_registered() const {
    return configuration.registered;
  }

  String get_wifi_ssid() const {
    return configuration.network_ssid;
  }

  String get_wifi_password() const {
    return configuration.network_password;
  }

  String get_mac_by_string() const {
    return configuration.mac_by_string;
  }

  String get_num_meters_to_install() const {
    return configuration.num_meters_to_install;
  }

  int get_status() const {
    return configuration.status;
  }

  String get_meter_serial(int index) const {
    if (index >= 0 && index < MAX_METERS) {
      return configuration.meter_serials[index];
    }
    return "";
  }

  String get_meter_address(int index) const {
    if (index >= 0 && index < MAX_METERS) {
      return configuration.meter_addresses[index];
    }
    return "";
  }
  void reset_configuration() {
    configuration = meter_communicator_config();
  }

  ConfigManager(const ConfigManager&) = delete;
  ConfigManager& operator=(const ConfigManager&) = delete;
};

ConfigManager* ConfigManager::instance = nullptr;
#endif  // CONFIG_MANAGER_H