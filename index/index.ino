#include <Arduino.h>
#include <M5Stack.h>
#include <esp_task_wdt.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <stdlib.h>

#include <NTPClient.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "nvs_flash.h"

#include "EEPROM.h"
#include "lib/defines.h"
#include "lib/ConfigManager.h"

#include "image/xbm.h"
#include "image/loginIndex.h"
#include "image/updateIndex.h"
#include "lib/writeAndReadEEPROM.h"
#include "lib/defines.h"

#include "src/modules/MeterSerialQuery.h"
#include "src/modules/CommunicatorManager.h"
#include "src/modules/EnergyMeterManager.h"
#include "src/modules/ModbusHandler.h"
#include "config/EthernetConfig.h"
#include "config/setting.h"
#include "CronAlarms.h"

TaskHandle_t Task1;
CronId id;

JSONVar myArrayPruebab4;
JSONVar myArray_Modbus;
JSONVar myArrayPrueba;

int parity = 1;
int initial_size = 0;

WiFiClientSecure client0;
EthernetClient client1;

WiFiUDP ntpUDP;


// NTPClient time_client_1(Udp, "0.pool.ntp.org", 36000, 60000);
NTPClient time_client_0(ntpUDP, "0.pool.ntp.org", 36000, 60000);

unsigned long previous_millis = 0;
const unsigned long interval = 1000;
unsigned long previousMillis_test = 0;
const unsigned long interval_test = 60000;
time_t epoch;

void increase_watchdog_time() {
  esp_task_wdt_init(10, true);
}

ConfigManager &config_manager = ConfigManager::getInstance();
EthernetConfig &ethernet_config = EthernetConfig::getInstance();
class meter_communicator {
private:

  ConfigureNetwork configuration_network;
  MeterSerialQuery meter_serial_query;
  CommunicatorManager communicator_manager;
  EnergyMeterManager energy_meter_manager;
  ModbusHandler modbus_handler;


  void load_configuration() {
    config_manager.update_field("project_id", memory_handler.get_data(handle, STORAGE, "project_id"));
    config_manager.update_field("communicator_id", memory_handler.get_data(handle, STORAGE, "communicator_id"));
    config_manager.update_field("operation_mode", memory_handler.get_data(handle, STORAGE, "operation_mode"));
    config_manager.update_field("registered", memory_handler.get_data(handle, STORAGE, "registered"));
    config_manager.update_field("status", memory_handler.get_data(handle, STORAGE, "status"));
    config_manager.update_field("num_meters_to_install", memory_handler.get_data(handle, STORAGE, "num_meters"));
    config_manager.update_field("wifi_ssid", memory_handler.get_data(handle, STORAGE, "wifi_ssid"));
    config_manager.update_field("wifi_pass", memory_handler.get_data(handle, STORAGE, "wifi_pass"));

    String meter_serial[MAX_METERS];
    String addresses[MAX_METERS];
    for (int i = 1; i <= MAX_METERS; i++) {
      String serial_meter = "serial_" + String(i);
      String address = "0" + String(i);
      meter_serial[i] = memory_handler.get_data(handle, STORAGE, serial_meter.c_str());
      addresses[i] = address;
    }
    config_manager.set_meter_addresses(addresses, MAX_METERS);
    config_manager.set_meter_serials(meter_serial, MAX_METERS);
  }

  void read_modbus_registers(String meter_index) {
    for (int i = 1; i <= meter_index.toInt(); i++) {
      String address = config_manager.get_meter_address(i);
      String meter_serial = config_manager.get_meter_serial(i);
      Serial.printf("address: %s\n", address);
      Serial.printf("serial: %s\n", meter_serial);
      modbus_handler.modbus_meter_query_handler(address, meter_serial);
      // consulta_medidor_modbus(address, meter_serial);
    }
  }

public:
  bool synchronize_with_ntp(NTPClient client) {
    M5.Lcd.clear();
    wifi_ap_record_t ap_info;

    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
      M5.Lcd.setTextColor(RED);
      M5.Lcd.println("WiFi no conectado");
    } else {
      M5.Lcd.println("WiFi no conectado");
    }

    for (int i = 0; i < 3; i++) {
      client.begin();
      client.setTimeOffset(-18000);

      bool status = client.update();
      if (status) {
        epoch = client.getEpochTime();
        Serial.println(epoch);

        struct tm *ptm = gmtime((time_t *)&epoch);
        timeval tv = { mktime(ptm), 0 };
        settimeofday(&tv, nullptr);

        Serial.print("Año sincronizado: ");
        Serial.println(ptm->tm_year + 1900);

        return true;
      }

      M5.Lcd.printf("Intento %d de sincronización fallido\n", i + 1);
      delay(1000);  // Esperar antes de reintentar
    }

    M5.Lcd.printf("Actualización de fecha fallida");
    return false;
    // struct tm *ptm = gmtime((time_t *)&epoch);
    // timeval tv = { mktime(ptm), 0 };
    // settimeofday(&tv, nullptr);
    // Serial.print("Year: ");
    // Serial.println(ptm->tm_year);
    // return true;
  }
  void setting_network() {
    configuration_network.set_network_configuration(config_manager.get_operation_mode().c_str());
  }
  void display_information() {
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Communicator Configuration:");
    M5.Lcd.print("Project ID: ");
    M5.Lcd.println(config_manager.get_project_id());
    delay(1000);
    M5.Lcd.print("Mode: ");
    M5.Lcd.println(config_manager.get_operation_mode());
    delay(1000);
    M5.Lcd.print("communicator_id: ");
    M5.Lcd.println(config_manager.get_communicator_id());
    delay(1000);
    M5.Lcd.print("registered: ");
    M5.Lcd.println(config_manager.get_registered());
    delay(1000);
    M5.Lcd.print("status: ");
    M5.Lcd.println(config_manager.get_status());
    delay(1000);
    M5.Lcd.print("num_meters_to_install: ");
    M5.Lcd.println(config_manager.get_num_meters_to_install());
    delay(1000);
    if (config_manager.get_wifi_ssid()) {
      M5.Lcd.print("network_ssid: ");
      M5.Lcd.println(config_manager.get_wifi_ssid());
      delay(1000);
      M5.Lcd.print("network_pass: ");
      M5.Lcd.println(config_manager.get_wifi_password());
    } else {
    }
    for (int i = 1; i <= config_manager.get_num_meters_to_install().toInt(); i++) {
      M5.Lcd.printf("Serial del Medidor: %s\n", config_manager.get_meter_serial(i));
    }
  }
  void initialize() {
    M5.begin();
    M5.Lcd.begin();
    while (!Serial) {
      ;
    }
    memory_handler.delete_nvs_memory(handle, STORAGE, "project_id");
    memory_handler.delete_nvs_memory(handle, STORAGE, "wifi_ssid");
    memory_handler.delete_nvs_memory(handle, STORAGE, "wifi_pass");
    memory_handler.delete_nvs_memory(handle, STORAGE, "operation_mode");
    memory_handler.delete_nvs_memory(handle, STORAGE, "registered");
    memory_handler.delete_nvs_memory(handle, STORAGE, "communicator_id");
    memory_handler.delete_nvs_memory(handle, STORAGE, "num_meters");
    memory_handler.delete_nvs_memory(handle, STORAGE, "status");
    memory_handler.delete_nvs_memory(handle, STORAGE, "serial_1");
    memory_handler.delete_nvs_memory(handle, STORAGE, "serial_2");
    memory_handler.delete_nvs_memory(handle, STORAGE, "serial_3");

    M5.lcd.setTextSize(2);
    delay(1000);
    Serial2.begin(9600);

    load_configuration();
    display_information();
    delay(5000);

    if (!ethernet_config.begin()) {
      Serial.println("Failed to initialize Ethernet!");
      while (1) {
        delay(1000);
      }
    }

    while (config_manager.get_status() == 404) {
      setting_network();
    }

    Serial.println("time zone setting starts");
    if (config_manager.get_operation_mode() == "1") {
      bool status = configuration_network.conexion(config_manager.get_wifi_ssid().c_str(), config_manager.get_wifi_password().c_str());
      synchronize_with_ntp(time_client_0);
    } else {
      M5.Lcd.println("config model ETH");
      Serial.println("Waiting for Ethernet connection...");
      while (!ethernet_config.isConnected()) {
        delay(500);
      }
      Serial.print("Dirección IP asignada: ");
      Serial.println(ethernet_config.get_ip_address());
    }

    int cont = config_manager.get_num_meters_to_install().toInt();
    Serial.printf("numero de medidores: %s\n", config_manager.get_num_meters_to_install());
    bool status = meter_serial_query.get_meter_serial(config_manager.get_num_meters_to_install());

    Serial.println(config_manager.get_communicator_id());
    Serial.println(config_manager.get_registered());


    if (config_manager.get_communicator_id() != "404"
        && config_manager.get_registered() == "404") {
      for (int i = 1; i <= MAX_METERS; i++) {
        if (config_manager.get_meter_serial(i) != "404"
            && config_manager.get_meter_serial(i) != "0"
            && config_manager.get_meter_serial(i) != "") {
          String serial_meter = config_manager.get_meter_serial(i);
          Serial.printf("serial: %s posision: %d\n", serial_meter, i);
          String response = energy_meter_manager.send_regiterd_meter(serial_meter);
          cont--;
          if (cont == 0) {
            config_manager.update_field("registered", "true");
            memory_handler.set_data(handle, STORAGE, "registered", "true");
          }
        }
      }
    } else if (config_manager.get_communicator_id() == "404"
               && config_manager.get_registered() == "404") {
      if (status) {
        String serial_1 = memory_handler.get_data(handle, STORAGE, "serial_1");
        communicator_manager.send_registered_communicator(serial_1);
        for (int i = 1; i <= MAX_METERS; i++) {
          Serial.printf("serial: %s posision: %d\n", config_manager.get_meter_serial(i), i);
          if (config_manager.get_meter_serial(i) != "404"
              && config_manager.get_meter_serial(i) != "0"
              && config_manager.get_meter_serial(i) != "") {
            String serial_meter = config_manager.get_meter_serial(i);
            Serial.printf("serial: %s posision: %d\n", serial_meter, i);
            String response = energy_meter_manager.send_regiterd_meter(serial_meter);
            cont--;

            if (cont == 0) {
              config_manager.update_field("registered", "true");
              memory_handler.set_data(handle, STORAGE, "registered", "true");
            }
          }
        }
      }
    } else {
      time_t t_now = time(nullptr);
      time_string = String(asctime(gmtime(&t_now)));
      read_modbus_registers(config_manager.get_num_meters_to_install());
    }
  }
};

meter_communicator communicator;


void setup() {
  communicator.initialize();

  // if (modo == "1") {
  //   M5.Lcd.setTextColor(WHITE);
  //   M5.Lcd.println("cargando datos de time: ");
  //   timeClient0.begin();
  //   timeClient0.setTimeOffset(-18000);
  //   timeClient0.update();
  //   epoch = timeClient0.getEpochTime();
  //   Serial.println(epoch);
  // } else {
  //   M5.Lcd.setTextColor(WHITE);
  //   M5.Lcd.println("cargando datos de time: ");
  //   timeClient1.begin();
  //   timeClient1.setTimeOffset(-18000);
  //   timeClient1.update();
  //   epoch = timeClient1.getEpochTime();
  //   Serial.println(epoch);
  // }

  // struct tm *ptm = gmtime((time_t *)&epoch);
  // timeval tv = { mktime(ptm), 0 };
  // settimeofday(&tv, nullptr);
  // Serial.print("Year: ");
  // Serial.println(ptm->tm_year);

  // if (ptm->tm_year <= 69) {
  //   // struct tm tm_newTime;
  //   // tm_newTime.tm_year = 2011 - 1900;
  //   // tm_newTime.tm_mon = 1 - 1;
  //   // tm_newTime.tm_mday = 1;
  //   // tm_newTime.tm_hour = 8;
  //   // tm_newTime.tm_min = 30;
  //   // tm_newTime.tm_sec = 0;
  //   // tm_newTime.tm_isdst = 0;

  //   // tv = { mktime(&tm_newTime), 0 };
  //   // settimeofday(&tv, nullptr);

  //   // time_t time_now = time(nullptr);
  //   // M5.Lcd.println(asctime(gmtime(&time_now)));
  //   M5.Lcd.println(gmtime((time_t *)&epoch));
  //   delay(30000);
  //   reset();
  // } else {
  //   M5.Lcd.println(gmtime((time_t *)&epoch));
  // }

  // M5.Lcd.println("Ending setup...");
  // delay(3000);

  // if (parity == 1) {
  //   Cron.create("0 15 0 * * *", reset, false);
  //   Cron.create("0 */5 * * * *", queryMeter, false);
  // }
  // pinMode(DE, OUTPUT);
  // pinMode(RE, OUTPUT);
  // M5.Lcd.begin();
}


void loop() {
  M5.update();
  M5.Lcd.setTextColor(WHITE);

  if (M5.BtnA.pressedFor(3000)) {
    M5.lcd.clear();
    M5.lcd.setCursor(0, 0);
    M5.Lcd.print("system data reset.");
    memory_handler.delete_nvs_memory(handle, STORAGE, "project_id");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "comunicator_id");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "operation_mode");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "registered");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "wifi_ssid");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "wifi_pass");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "serial_1");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "serial_2");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "serial_3");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "serial_4");
    delay(1000);
    M5.Lcd.print(".");
    memory_handler.delete_nvs_memory(handle, STORAGE, "serial_5");
    reset();
  }

  unsigned long current_millis = millis();
  if (current_millis - previous_millis >= interval) {
#ifdef __AVR__
    system_tick();  // must be implemented at 1Hz
#endif
    time_t t_now = time(nullptr);

    time_string = String(asctime(gmtime(&t_now)));
    if (minutes != time_string.substring(15, 16)) {
      minutes = time_string.substring(15, 16);
      M5.Lcd.fillScreen(0);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Horus Smart Energi...");
      String date = time_string.substring(0, 16);
      String year = time_string.substring(20);
      date = date + " : " + year;
      M5.Lcd.drawString(date, 0, 60, 1);

      if (config_manager.get_operation_mode() == "1") {
        if (WiFi.status() != WL_CONNECTED) {
          M5.Lcd.print("estado de la red: ");
          M5.Lcd.drawCircle(220, 25, 7, WHITE);
          M5.Lcd.fillCircle(220, 25, 6, RED);
          M5.Lcd.println(WiFi.status() != WL_CONNECTED);
          M5.Lcd.println("Reconnecting to WiFi...");
          WiFi.disconnect();
          WiFi.reconnect();
        } else {
          M5.Lcd.print("estado de la red: ");
          M5.Lcd.drawCircle(220, 25, 7, WHITE);
          M5.Lcd.fillCircle(220, 25, 6, GREEN);
        }
        M5.Lcd.drawXBitmap(axis_x, axis_y, logo, logo_width, logo_height, TFT_WHITE, TFT_BLACK);
      } else {
        if (Ethernet.linkStatus() == Unknown) {
          M5.Lcd.println("Ethernet status unknown");
        } else if (Ethernet.linkStatus() == LinkON) {
          M5.Lcd.println("Ethernet status: On");
          M5.Lcd.drawCircle(240, 25, 7, WHITE);
          M5.Lcd.fillCircle(240, 25, 6, GREEN);
        } else if (Ethernet.linkStatus() == LinkOFF) {
          M5.Lcd.println("Ethernet status: Off");
          M5.Lcd.drawCircle(245, 25, 7, WHITE);
          M5.Lcd.fillCircle(245, 25, 6, RED);
        }
        M5.Lcd.drawXBitmap(axis_x, axis_y, logo, logo_width, logo_height, TFT_WHITE, TFT_BLACK);
      }
    }

    Cron.delay();
    previous_millis = current_millis;
  }

  esp_task_wdt_reset();
}

void reset() {
  ESP.restart();
}
