#include "stdlib_noniso.h"
#include "esp_err.h"
#include "HardwareSerial.h"
#ifndef EthernetConfig_h
#define EthernetConfig_h

#include <SPI.h>
#include <EthernetLarge.h>
#include <ESPmDNS.h>

#include "../lib/defines.h"

class EthernetConfig {
private:
  static EthernetConfig* instance;
  EthernetClient eth_client;
  boolean eth_connected;
  unsigned long startMillis;
  void get_ethernet_hardware(int status) {
    switch (status) {
      case EthernetNoHardware:
        Serial.println("No se detectó hardware Ethernet.");
        break;

      case EthernetW5100:
        Serial.println("Hardware detectado: W5100.");
        break;

      case EthernetW5200:
        Serial.println("Hardware detectado: W5200.");
        break;

      case EthernetW5500:
        Serial.println("Hardware detectado: W5500.");
        break;

      default:
        Serial.println("Estado del hardware desconocido.");
        break;
    }
  }

  void get_ethernet_status(int status) {
    switch (status) {
      case Unknown:
        Serial.println("Estado del enlace Ethernet: Desconocido.");
        break;

      case LinkON:
        Serial.println("Estado del enlace Ethernet: Conectado.");
        break;

      case LinkOFF:
        Serial.println("Estado del enlace Ethernet: Desconectado.");
        break;

      default:
        Serial.println("Estado no reconocido.");
        break;
    }
  }

  EthernetConfig() {
    eth_connected = false;
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, -1);
    Ethernet.init(CS_PIN);
  }

  EthernetConfig(const EthernetConfig&) = delete;
  EthernetConfig& operator=(const EthernetConfig&) = delete;

public:
  static EthernetConfig& getInstance() {
    if (instance == nullptr) {
      instance = new EthernetConfig();
    }
    return *instance;
  }

  bool begin() {
    startMillis = millis();
    if (Ethernet.begin(mac) == 0) {
      Serial.println("no inicia modulo");
    }
    EthernetHardwareStatus hwStatus = Ethernet.hardwareStatus();
    get_ethernet_hardware(hwStatus);

    EthernetLinkStatus eth_status = Ethernet.linkStatus();
    get_ethernet_status(eth_status);

    eth_client.connected();

    eth_connected = true;
    delay(1000);
    return eth_connected;
  }

  bool isConnected() {
    return eth_connected;
  }

  IPAddress get_ip_address() {
    return Ethernet.localIP();
  }

  IPAddress get_ip_Gateway() {
    return Ethernet.gatewayIP();
  }

  IPAddress get_ip_DNS() {
    return Ethernet.dnsServerIP();
  }

  String http_get_req(const char* server_path, const char* path, const char* serial_number_meter) {
    EthernetClient client;
    String url = String(path) + String(serial_number_meter);
    if (client.connect(server_path, PORT)) {
      client.print(String("GET ") + url + " HTTP/1.1\r\n"
                   + "Host: " + server_path + " \r\n" + "Authorization: Bearer "
                   + api_key + "\r\n" + "Content-Type: application/json\r\n\r\n");
      delay(1000);
      String line;
      while (client.available() == 0)
        ;
      while (client.available()) {
        line = client.readStringUntil('\n');
      }
      int pos = line.indexOf("body");
      client.println("Connection: close");
      client.println();
      Serial.println(line);
      return line;
    }
  }

};

EthernetConfig* EthernetConfig::instance = nullptr;
#endif
