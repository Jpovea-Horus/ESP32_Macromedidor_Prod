#include "esp32-hal.h"
#include "HardwareSerial.h"
#include <M5Stack.h>
#include "WString.h"
#include "pgmspace.h"
#include "Arduino.h"
#include "ConversionUtils.h"
#include "CrcCalculator.h"
#include "SerialValidator.h"
#include "../../lib/defines.h"
#include "../../lib/writeAndReadEEPROM.h"
#include "../../lib/ConfigManager.h"
#ifndef METER_SERIAL_QUERY_H
#define METER_SERIAL_QUERY_H



class MeterSerialQuery {
private:
  ConfigManager& config_manager = ConfigManager::getInstance();
  String serials[MAX_METERS];
  ConversionUtils conversion_utils;
  CrcCalculator crc_calculator;
  meter_communicator_config configuration;
  MemoryHandler memory_handler;
  SerialDBQuery serial_db_query_handler;

  byte plots_serial_modbus[8];
  bool is_meter_present = false;

  void send_modbus_data(byte* plots_Modbus) {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    Serial2.write(plots_Modbus, 8);
  }

  void decode_data_modbus(String frame, byte* output) {
    const int number_values = 8;

    for (int i = 0; i < number_values; i++) {
      String value_string = frame.substring(i * 2, i * 2 + 2);
      output[i] = strtoul(value_string.c_str(), nullptr, 16);
    }
  }

  String check_serial_modbus() {
    String response;
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    int received_byte_modbus = Serial2.available();
    byte BufferValue[received_byte_modbus];
    for (int i = 0; i < received_byte_modbus; i++) {
      BufferValue[i] = Serial2.read();
    }
    for (int i = 0; i < received_byte_modbus; i++) {
      response += conversion_utils.dec_to_hex(BufferValue[i], 2);
    }
    return response;
  }

  String query_meter_serial(String meter_addresses) {
    String address = meter_addresses + "03FC000002";

    String serial_checksum = crc_calculator.cyclic_redundancy_check(address);
    String frame = address + serial_checksum;

    int query_attempts = 3;
    String response;

    while (query_attempts > 0) {
      decode_data_modbus(frame, plots_serial_modbus);
      send_modbus_data(plots_serial_modbus);
      delay(10);

      response = check_serial_modbus();
      if (response.length() == 18) {
        return String(conversion_utils.hex_to_dec(response.substring(6, 14)));
      }
      query_attempts--;
      delay(600);
    }
    return "";
  }

  String* generate_meter_addresses(String num_meters_to_install, int& size) {
    int number_meters = num_meters_to_install.toInt();

    String* meter_addresses = new String[number_meters];

    size = number_meters;

    for (int i = 0; i < number_meters; i++) {
      if (i + 1 < 10) {
        meter_addresses[i] = "0" + String(i + 1);
      } else {
        meter_addresses[i] = String(i + 1);
      }
    }

    return meter_addresses;
  }


public:
  bool get_meter_serial(String num_meters_to_install) {
    int num_addresses;
    String* addresses = generate_meter_addresses(num_meters_to_install, num_addresses);
    for (int i = 0; i < num_addresses; i++) {
      String serial_meter = query_meter_serial(addresses[i].c_str());
      Serial.println(serial_meter);
      serials[i+1] = serial_meter;
      String serial_key = "serial_" + String(i + 1);
      memory_handler.set_data(handle, STORAGE, serial_key.c_str(), serial_meter.c_str());
      M5.begin();
      M5.Lcd.begin();
      M5.Lcd.printf("Meter Serial: %s\n", serial_meter);
      is_meter_present = serial_db_query_handler.check_serial_in_db(serial_meter);
      if (is_meter_present) {
        M5.Lcd.println("Meter enabled");
      }
      delay(10000);
    }
    config_manager.set_meter_serials(serials, MAX_METERS);
    return is_meter_present;
  }
};

#endif