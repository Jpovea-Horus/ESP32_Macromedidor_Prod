#include "lib/converters.h"
#include <WiFi.h>

unsigned long previousMillisOne = 0;
const long intervalOne = 10000;

void configureRed() {
  if (modo == "") {
    M5.lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("configuracion inicial");
    Serial.println("configuracion inicial");
    delay(5000);
    M5.lcd.clear();
    menuinicial();
  }
  if (modo == "1") {
    if (ssidString == "" && passwordString == "") {
      M5.lcd.clear();
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.println("configura una red wifi");
      Serial.println("configura una red wifi");
      delay(5000);
      menu(2);
    } else {
      ssid = converterStrcChar(ssidString);
      password = converterStrcChar(passwordString);
    }
  } else if (modo == "0") {
    if (macbystring == "") {
      M5.lcd.clear();
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("se configura en una red LAN");
      delay(3000);
    } else {
      str2mac(str, mac);
      LAN = 1;
    }
  }

  if (ssidString != "" && passwordString != "") {
    int contador = 1;
    M5.lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("connecting to wifi");
    Serial.println("connecting to wifi");
    M5.Lcd.setTextColor(YELLOW);
    Serial.println(ssidString);
    // Serial.println(passwordString);
    M5.Lcd.println(ssid);
    M5.Lcd.println("******");
    delay(2000);
    bool statusConect = conexion();
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      M5.Lcd.setTextColor(RED);
      Serial.println(WiFi.waitForConnectResult());
      M5.Lcd.println("WiFi Failed!\n");
      Serial.println("WiFi Failed!\n");
      M5.Lcd.println("verifique el SSD y Password");
      Serial.println("verifique el SSD y Password");
      delay(6000);
    } else {
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.println("conectado a: ");
      Serial.println("conectado a: ");
      M5.Lcd.setTextColor(GREEN);
      Serial.println(ssid);
      M5.Lcd.println(ssid);
      delay(1000);
    }
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print("ip adress: ");
    Serial.println("ip adress: ");
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.println(WiFi.localIP());
    Serial.println(WiFi.localIP());
    delay(6000);
    config();
  } else if (mac1 != 0 && LAN == 1) {
    M5.lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Inicia ethernet por DHCP");
    SPI.begin(SCK, MISO, MOSI, -1);
    Ethernet.init(CS);
    if (Ethernet.begin(mac1) == 0) {
      M5.Lcd.println("Failed to configure Ethernet using DHCP");
      Ethernet.begin(mac1, ip);
    }
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1);  // do nothing, no point running without Ethernet hardware
      }
      if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
      }
      Ethernet.begin(mac1, ip);
    } else {
      M5.Lcd.println("connection LAN ip:");
      M5.Lcd.println(Ethernet.localIP());
      Serial.println(Ethernet.subnetMask());
      Serial.println(Ethernet.localIP());
      Udp.begin(Ethernet.localIP());
    }
    delay(6000);
    config();
  }
}

void config() {
  if (projectId == 0) {
    M5.lcd.clear();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("codigo de projecto");
    Serial.println("codigo de projecto");
    delay(2000);
    menu(0);
    if (nElements == 0) {
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.println("medidores a instalar");
      delay(2000);
      menu(1);
    } else {
      String dato = readStringFromEEPROM(20);
      if(dato == "3"){
        numberMeters[0] = "01";
        numberMeters[1] = "02";
        numberMeters[2] = "03";

      } else if (dato == "2") {
        numberMeters[0] = "01";
        numberMeters[1] = "02";
      } else {
        numberMeters[0] = "01";
      }
    }

    if (serialUnoString == "") {
      M5.Lcd.println("#Se require serial de medidores");
      delay(3000);
      M5.lcd.clear();
      String respuesta = sendSerialNumber();
      M5.Lcd.begin();
      delay(1000);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.println("seriales de los medidores: ");
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.println(respuesta);
      Serial.println(respuesta);
    } else {
      medidorUnoModbus = serialUnoString;
      medidorDosModbus = serialDosString;
      medidorTresModbus = serialTresString;
    }
  } else {
    String keyId = readStringFromEEPROM(0);
    projectId = keyId.toInt();
    String dato = readStringFromEEPROM(20);
    if (dato == "3") {
      numberMeters[0] = "01";
      numberMeters[1] = "02";
      numberMeters[2] = "03";
    } else if (dato == "2") {
      numberMeters[0] = "01";
      numberMeters[1] = "02";
    } else {
      numberMeters[0] = "01";
    }
    medidorUnoModbus = serialUnoString;
    medidorDosModbus = serialDosString;
    medidorTresModbus = serialTresString;
  }
  status = 0;
}

bool conexion() {
  WiFi.begin(ssid, password);
  unsigned long currentMillisOne = millis();
  previousMillisOne = currentMillisOne;
  while (status != WL_CONNECTED) {
    currentMillisOne = millis();
    if (currentMillisOne - previousMillisOne >= intervalOne) {
      return false;
    }
  }
  return true;
}