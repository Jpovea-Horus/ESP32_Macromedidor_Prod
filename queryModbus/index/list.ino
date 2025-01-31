#include "lib/converters.h"
#include "lib/writeAndReadEEPROM.h"

int datoMenuInicial = 0;
int eepromsiza = 0;
int item;
String key;

void menu(int dato) {
  item = dato;
  switch (item) {
    case 0:
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.print("Ingrese codigo del proyecto");
      Serial.print("#Ingrese codigo del proyecto: ");
      key = frase();
      projectId = key.toInt();
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.println(projectId);
      Serial.println(projectId);
      if (EEPROM.read(0) == 0) {
        writeStringToEEPROM(0, key);
      }
      break;
    case 1:
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.println("Ingrese la cantidad de medidores: ");
      Serial.println("#Ingrese la cantidad de medidores: ");
      key = frase();
      writeStringToEEPROM(20, key);
      if (EEPROM.read(20) > 0) {
        String dato = readStringFromEEPROM(20);
        if (dato == "3") {
          numberMeters[0] = "01";
          numberMeters[1] = "02";
          numberMeters[2] = "03";
          M5.Lcd.setTextColor(WHITE);
          M5.Lcd.println("Direccion de medidores: ");
          Serial.print("##Direccion de medidores: ");
          M5.Lcd.setTextColor(GREEN);
          M5.Lcd.print(numberMeters[0]);
          Serial.print(numberMeters[0]);
          M5.Lcd.print(", ");
          Serial.println(", ");
          M5.Lcd.println(numberMeters[1]);
          Serial.println(numberMeters[1]);
          M5.Lcd.print(", ");
          Serial.println(", ");
          M5.Lcd.print(numberMeters[2]);
          Serial.println(numberMeters[2]);
          M5.Lcd.setTextColor(YELLOW);
        } else if (dato == "2") {
          numberMeters[0] = "01";
          numberMeters[1] = "02";
          M5.Lcd.setTextColor(WHITE);
          M5.Lcd.println("Direccion de medidores: ");
          Serial.print("##Direccion de medidores: ");
          M5.Lcd.setTextColor(GREEN);
          M5.Lcd.print(numberMeters[0]);
          Serial.print(numberMeters[0]);
          M5.Lcd.print(", ");
          Serial.println(", ");
          M5.Lcd.println(numberMeters[1]);
          Serial.println(numberMeters[1]);
          M5.Lcd.setTextColor(YELLOW);
        } else if (dato == "1") {
          numberMeters[0] = "01";
          M5.Lcd.setTextColor(WHITE);
          M5.Lcd.print("Direccion de medidores: ");
          Serial.print("##Direccion de medidores: ");
          M5.Lcd.setTextColor(GREEN);
          M5.Lcd.println(numberMeters[0]);
          Serial.println(numberMeters[0]);
          M5.Lcd.setTextColor(YELLOW);
        }
        delay(3000);
      }
      break;
    case 2:
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.println("##SSID de tu red: ");
      Serial.print("##SSID de tu red: ");
      key = frase();
      ssidString = key;
      ssid = converterStrcChar(key);
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.println(ssidString);
      Serial.println(ssidString);
      writeStringToEEPROM(70, key);
      eepromsiza = EEPROM.read(70);
      if (password == "") {
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.print("##password: ");
        Serial.print("##password: ");
        key = frase();
        passwordString = key;
        password = converterStrcChar(key);
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.println("*************");
        Serial.println("*************");
        if (EEPROM.read(90) == 0) {
          writeStringToEEPROM(90, key);
        }
      }
      delay(5000);
      break;
  }
}

void menuinicial() {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("1. Red WiFi");
  Serial.println("1. Red WiFi");
  M5.Lcd.drawCircle(155, 7, 7, WHITE);
  M5.Lcd.println("2. Red LAN");
  Serial.println("2. Red LAN");
  M5.Lcd.drawCircle(155, 26, 7, WHITE);
  M5.Lcd.drawXBitmap(axisx, axisy, logo, logoWidth, logoHeight, TFT_WHITE, TFT_BLACK);
  datoMenuInicial = valor();
  switch (datoMenuInicial) {
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
      modo = "1";
      writeStringToEEPROM(120, modo);
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
      modo = "0";
      writeStringToEEPROM(120, modo);
      writeStringToEEPROM(130, key);
      break;
  }
}

int valor() {
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

String frase() {
  String inString = "";
  // Limpiar el buffer del puerto serie antes de leer la entrada
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
