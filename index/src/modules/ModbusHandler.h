#include <HardwareSerial.h>

#include <stdio.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../lib/tramasModbus.h"
#include "../../lib/ConfigManager.h"
#include "NetworkHandler.h"
#include "ConversionUtils.h"
#include "CrcCalculator.h"
#include "ModbusVariableConverter.h"




#ifndef MODBUS_HANDLER_H
#define MODBUS_HANDLER_H

class ModbusHandler {
private:
  String name_var;
  ConfigManager& config_manager = ConfigManager::getInstance();

  NetworkHandler network_handler;
  ConversionUtils conversion_utils;
  CrcCalculator crc_calculator;
  ModbusVariableConverter modbus_var_con;

  JSONVar myArrayPrueba;
  JSONVar modbus_data_array;
  JSONVar error_logs;

  uint8_t modbus_bytes[8];

  String modbus_frames[34];
  String modbus_raw_value;
  String http_response_data;

  int remaining_elements;
  int modbus_default_value = 3;
  int current_query_index = 0, retry_count = 0, total_queries = 0;

  String read_serial_modbus() {
    String modbus_hex_data;
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    int received_byteModbus = Serial2.available();
    byte BufferValue[received_byteModbus];
    for (int i = 0; i < received_byteModbus; i++) {
      BufferValue[i] = Serial2.read();
    }
    for (int i = 0; i < received_byteModbus; i++) {
      modbus_hex_data += conversion_utils.dec_to_hex(BufferValue[i], 2);
    }
    return modbus_hex_data;
  }

  void decode_modbus_data(String datos) {
    const int numValues = 8;
    for (int i = 0; i < numValues; i++) {
      String valueStr = datos.substring(i * 2, i * 2 + 2);
      modbus_bytes[i] = (uint8_t)strtoul(valueStr.c_str(), nullptr, 16);
    }
  }

  void send_modbus_data() {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    Serial2.write(modbus_bytes, 8);
  }

  String add_leading_zeros(String binaryNumber, int desiredLength) {
    int currentLength = binaryNumber.length();
    if (currentLength >= desiredLength) {
      return binaryNumber;
    }
    int numZeros = desiredLength - currentLength;
    for (int i = 0; i < numZeros; i++) {
      binaryNumber = "0" + binaryNumber;
    }
    return binaryNumber;
  }

  int parse_binary_to_float(String binStr) {
    int val = 0;
    for (int i = 0; i < binStr.length(); i++) {
      val = val * 2 + (binStr[i] - 48);
    }
    return val;
  }

  void handle_error(const String& error_message, const String& meter_serial){
     
  }

public:
  void init() {
    Serial.printf("status connection net: %d\n", WL_CONNECTED);
    network_handler.init(server_local.c_str(), server_lan.c_str(), api_key);
  }

  void modbus_meter_query_handler(String& address, String& meter_serial) {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
      network_handler.set_connection_type(true);
    } else {
      Serial.printf("No conectado a Wi-Fi.\n");
      network_handler.set_connection_type(false);
    }
    do {
      String plot = address + VariablesModbus[current_query_index];
      String CRC_16 = crc_calculator.cyclic_redundancy_check(plot);
      modbus_frames[current_query_index] = plot + CRC_16;
      decode_modbus_data(modbus_frames[current_query_index]);
      send_modbus_data();
      modbus_raw_value = read_serial_modbus();

      if (modbus_raw_value.length() == 18) {
        delay(60);
        modbus_raw_value = modbus_raw_value.substring(6, 14);
        myArrayPrueba[current_query_index] = "value: " + modbus_raw_value;
        String binaryNumer0 = add_leading_zeros(conversion_utils.hex_to_bin(modbus_raw_value.substring(0, 2)), 8);
        String binaryNumer1 = add_leading_zeros(conversion_utils.hex_to_bin(modbus_raw_value.substring(2, 4)), 8);
        String binaryNumer2 = add_leading_zeros(conversion_utils.hex_to_bin(modbus_raw_value.substring(4, 6)), 8);
        String binaryNumer3 = add_leading_zeros(conversion_utils.hex_to_bin(modbus_raw_value.substring(6, 8)), 8);

        String Binaryvalue = binaryNumer0 + binaryNumer1 + binaryNumer2 + binaryNumer3;
        int sizeBinaryvalue = Binaryvalue.length();
        int exponent = parse_binary_to_float(Binaryvalue.substring(1, 9)) - 127;

        String mantissa = (exponent <= -1) ? Binaryvalue.substring(9) : "1" + Binaryvalue.substring(9);
        String mantissaEntero = mantissa.substring(0, exponent + 1);
        String mantissaDecimal = mantissa.substring(exponent + 1);

        float NumeroEntero = 0;
        float NumeroDecimal = 0;

        int ExponentEntero = exponent;
        int ExponentDecimal = -1;
        for (int i = 0; i < mantissaEntero.length(); i++) {
          int element = mantissaEntero[i] - '0';
          NumeroEntero += element * pow(2, ExponentEntero);
          ExponentEntero--;
        }
        for (int i = 0; i < mantissaDecimal.length(); i++) {
          int element = mantissaDecimal[i] - '0';
          NumeroDecimal += element * pow(2, ExponentDecimal);
          ExponentDecimal--;
        }
        int posicion = current_query_index - 1;
        if (posicion == -1) {
          modbus_var_con.convert_modbus_variable_name(modbus_frames[34]);
        } else {
          modbus_var_con.convert_modbus_variable_name(modbus_frames[posicion]);
        }
        float ValueTotal = NumeroEntero + NumeroDecimal;

        modbus_data_array[current_query_index] = name_var + ": " + String(ValueTotal);

        if (current_query_index == 34) {
          M5.Lcd.begin();
          current_query_index = 0;
          total_queries = 0;
          remaining_elements--;
          posicion = 0;
          modbus_data_array[current_query_index];
          modbus_data_array[35] = time_string;
          String jsonStringModbus = JSON.stringify(modbus_data_array);
          http_response_data = network_handler.http_post_req(path.c_str(), jsonStringModbus.c_str(), meter_serial.c_str());
          Serial.println(http_response_data);
          retry_count = 0;
        } else {
          current_query_index++;
          total_queries++;
        }
      } else {
        error_logs[current_query_index] = "incorrect data, the query is sent again: " + String(modbus_frames[current_query_index]);
        String jsonStrinCRC = JSON.stringify(error_logs);
        retry_count++;
        total_queries++;
        delay(500);
        if (retry_count == 10) {
          M5.Lcd.begin();
          String jsonError = "";
          total_queries = 0;
          retry_count = 0;
          JsonDocument doc;
          doc["message"] = "exceeded the maximum number of attempts";
          doc["Time"] = time_string.substring(0, 24);
          serializeJson(doc, jsonError);
          delay(1000);

          String datos = "";
          doc["error_path"] = error_path;
          doc["jsonError"] = jsonError;
          doc["meter_serial"] = meter_serial;

          serializeJson(doc, datos);
          delay(5000);
          http_response_data = network_handler.http_post_req(error_path.c_str(), jsonError.c_str(), meter_serial.c_str());
          M5.lcd.print(http_response_data);
          delay(5000);
        }
      }
      delay(600);
      esp_task_wdt_reset();
    } while (total_queries > 0);
    return;
  }
};

#endif