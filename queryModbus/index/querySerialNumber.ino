#include "lib/converters.h"
#include "lib/writeAndReadEEPROM.h"
StaticJsonDocument<300> req;
String trama[2];
byte plotsModbus[8];

String direccion;
String direccionDos;
String direccionUno;
String crcSerialNumber = "";

int i = 0;
int xz = 0;
String num = "";
int numberquery = 0;
String seriales = "";
String crcSerialNumberUno = "";
String crcSerialNumberDos = "";

String sendSerialNumber() {
  if (numberMeters[2] != "") {
    do {
      Serial.print("numero de repeticiones: ");
      Serial.println(i);
      if (i >= 3) {
        Serial.println("numero 3");
      } else {
        direccion = numberMeters[i] + "03FC000002";
        crcSerialNumber = CyclicRedundancyCheck(direccion);
        tramas[i] = direccion + crcSerialNumber;
        DecodeDataModbus(tramas[i]);
        enviardatoModbus();
      }
      num = serialCheckModbus();
      if (num.length() == 18) {
        int posicion = i - 1;
        if (posicion == 0) {
          serialUnoString = String(hexToDec(num.substring(6, 14)));
          writeStringToEEPROM(30, serialUnoString);
          Serial.println(serialUnoString);
          req["serialMedidorOne"] = String(hexToDec(num.substring(6, 14)));
        } else if (posicion == 1) {
          serialDosString = String(hexToDec(num.substring(6, 14)));
          writeStringToEEPROM(40, serialDosString);
          Serial.println(serialDosString);
          req["serialMedidorDos"] = String(hexToDec(num.substring(6, 14)));
        } else if (posicion == 2) {
          serialTresString = String(hexToDec(num.substring(6, 14)));
          writeStringToEEPROM(50, serialTresString);
          Serial.println(serialTresString);
          req["serialMedidorTres"] = String(hexToDec(num.substring(6, 14)));
        }
        Serial.print("posicion: ");
        Serial.println(posicion);
        if (i == 4) {
          numberquery = 0;
          i = 0;
        } else {
          i++;
          numberquery++;
        }
      } else {
        Serial.println("data null");
        numberquery++;
        xz++;
        if (xz == 4) {
          numberquery = 0;
          xz = 0;
        }
      }
      delay(600);
    } while (numberquery > 0);
    serializeJson(req, seriales);
    M5.Lcd.println(seriales);
  } else if (numberMeters[1] != "") {
    do {
      Serial.print("numero de repeticiones: ");
      Serial.println(i);
      if (i >= 2) {
        Serial.println("numero 2");
      } else {
        direccion = numberMeters[i] + "03FC000002";
        crcSerialNumber = CyclicRedundancyCheck(direccion);
        tramas[i] = direccion + crcSerialNumber;
        DecodeDataModbus(tramas[i]);
        enviardatoModbus();
      }
      num = serialCheckModbus();
      if (num.length() == 18) {
        int posicion = i - 1;
        if (posicion == 0) {
          serialUnoString = String(hexToDec(num.substring(6, 14)));
          writeStringToEEPROM(30, serialUnoString);
          Serial.println(serialUnoString);
          req["serialMedidorOne"] = String(hexToDec(num.substring(6, 14)));
        } else if (posicion == 1) {
          serialDosString = String(hexToDec(num.substring(6, 14)));
          writeStringToEEPROM(40, serialDosString);
          Serial.println(serialDosString);
          req["serialMedidorDos"] = String(hexToDec(num.substring(6, 14)));
        } 
        if (i == 2) {
          numberquery = 0;
          i = 0;
        } else {
          i++;
          numberquery++;
        }
      } else {
        Serial.println("data null");
        numberquery++;
        xz++;
        if (xz == 2) {
          numberquery = 0;
          xz = 0;
        }
      }
      delay(600);
    } while (numberquery > 0);
    serializeJson(req, seriales);
    M5.Lcd.println(seriales);
  } else if (numberMeters[0] != "") {
    direccionUno = numberMeters[0] + "03FC000002";
    crcSerialNumberUno = CyclicRedundancyCheck(direccionUno);
    tramas[0] = direccionUno + crcSerialNumberUno;
    do {
      DecodeDataModbus(tramas[0]);
      enviardatoModbus();
      delay(10);
      num = serialCheckModbus();
      if (num.length() == 18) {
        serialUnoString = String(hexToDec(num.substring(6, 14)));
        Serial.println(serialUnoString);
        writeStringToEEPROM(30, serialUnoString);
        req["serialMedidorUno"] = String(hexToDec(num.substring(6, 14)));
        serializeJson(req, seriales);
        numberquery = 0;
      } else {
        numberquery++;
        if (numberquery == 3) {
          numberquery = 0;
        }
      }
      delay(600);
    } while (numberquery > 0);
  }
  if (seriales == "") {
    M5.Lcd.print("no se encontro medidor");
    return "null";
  } else {
    return seriales;
  }
}

void DecodeDataModbus(String datos) {
  const int numValues = 8;  // Número de valores a extraer
  for (int i = 0; i < numValues; i++) {
    String valueStr = datos.substring(i * 2, i * 2 + 2);
    plotsModbus[i] = strtoul(valueStr.c_str(), nullptr, 16);
  }
}


void enviardatoModbus() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  Serial2.write(plotsModbus, 8);
}

String serialCheckModbus() {
  String x;
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  int received_byteModbus = Serial2.available();
  byte BufferValue[received_byteModbus];
  for (int i = 0; i < received_byteModbus; i++) {
    BufferValue[i] = Serial2.read();
  }
  for (int i = 0; i < received_byteModbus; i++) {
    x += decToHex(BufferValue[i], 2);
  }
  return x;
}
