
#include "lib/CRC.h"
#include "lib/tramasModbus.h"
#include "network/packagesent.h"
#include <stdio.h>
unsigned long previousmillis0 = 0;
const unsigned long interval1 = 1000;
String sensorReadings;
String tramas[34];
byte plots_Modbus[8];
int wz = 3;
int a = 0, b = 0, numberqueries = 0;
String val2;
size_t received_byteModbus = 0;
String NameVariable;


void ConsultaMedidorModbus(String address) {
  Serial.println(address);
  unsigned long currentMillis = millis();
  do {
    String plot = address + VariablesModbus[a];
    String CRC_16 = CyclicRedundancyCheck(plot);
    tramas[a] = plot + CRC_16;
    DecodeData_modbus(tramas[a]);
    enviardato_Modbus();
    val2 = serialCheck_Modbus();
    // myArrayPruebab1[a] = "lenght data: " + String(val2.length()) + " ,";

    if (val2.length() == 18) {
      delay(60);
      val2 = val2.substring(6, 14);
      myArrayPrueba[a] = "value: " + val2;
      String binaryNumer0 = addLeadingZeros(hexToBin(val2.substring(0, 2)), 8);
      String binaryNumer1 = addLeadingZeros(hexToBin(val2.substring(2, 4)), 8);
      String binaryNumer2 = addLeadingZeros(hexToBin(val2.substring(4, 6)), 8);
      String binaryNumer3 = addLeadingZeros(hexToBin(val2.substring(6, 8)), 8);

      /* se unie el numero en binario*/
      String Binaryvalue = binaryNumer0 + binaryNumer1 + binaryNumer2 + binaryNumer3;
      int sizeBinaryvalue = Binaryvalue.length();
      int exponent = convertir(Binaryvalue.substring(1, 9)) - 127;

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
      int posicion = a - 1;
      if (posicion == -1) {
        NameVariableModbus(tramas[34]);
      } else {
        NameVariableModbus(tramas[posicion]);
      }
      float ValueTotal = NumeroEntero + NumeroDecimal;
      //--------------------------------------------------------
      myArray_Modbus[a] = NameVar + ": " + String(ValueTotal);

      if (a == 34) {
        M5.Lcd.begin();
        a = 0;
        numberqueries = 0;
        nElements--;
        posicion = 0;
        myArray_Modbus[a];
        myArray_Modbus[35] = Time;
        String jsonStringModbus = JSON.stringify(myArray_Modbus);

        if (address == "03") {
          meterSerial = serialTresString;
          sensorReadings = httpPOSTReq(server, serverLan, path, jsonStringModbus, meterSerial);
          Serial.println(sensorReadings);
          // StaticJsonDocument<300> req;
          // DeserializationError Error = deserializeJson(req, sensorReadings);
          // if (Error) {
          //   Serial.print("Deserialization failed with code");
          //   Serial.println(Error.f_str());
          // } else {
          //   sensorReadings = req["body"];
          // }
          // Serial.println(sensorReadings);
        }
        if (address == "02") {
          meterSerial = serialDosString;
          sensorReadings = httpPOSTReq(server, serverLan, path, jsonStringModbus, meterSerial);
          // StaticJsonDocument<300> req;
          // DeserializationError Error = deserializeJson(req, sensorReadings);
          // if (Error) {
          //   Serial.print("Deserialization failed with code");
          //   Serial.println(Error.f_str());
          // } else {
          //   sensorReadings = req["body"];
          // }
          // Serial.println(sensorReadings);
        }
        if (address == "01") {
          String datos = "";
          meterSerial = serialUnoString;
          sensorReadings = httpPOSTReq(server, serverLan, path, jsonStringModbus, meterSerial);

          //   StaticJsonDocument<300> req;
          //   DeserializationError Error = deserializeJson(req, sensorReadings);
          //   if (Error) {
          //     Serial.print("Deserialization failed with code");
          //     Serial.println(Error.f_str());
          //   } else {
          //     sensorReadings = req["body"];
          //   }
          Serial.println(sensorReadings);
          // }
        }
        b = 0;
        // bot.sendMessage(CHAT_ID, sensorReadings, "");
      } else {
        a++;
        numberqueries++;
      }
    } else {
      myArrayPruebab4[a] = "incorrect data, the query is sent again: " + String(tramas[a]);
      String jsonStrinCRC = JSON.stringify(myArrayPruebab4);
      b++;
      numberqueries++;
      delay(500);
      if (b == 10) {
        M5.Lcd.begin();
        String jsonError = "";
        numberqueries = 0;
        b = 0;
        StaticJsonDocument<300> doc;
        doc["message"] = "exceeded the maximum number of attempts";
        doc["Time"] = Time.substring(0, 24);
        serializeJson(doc, jsonError);
        delay(1000);
        if (address == "01") {
          String datos = "";
          StaticJsonDocument<300> doc;
          doc["server"] = server;
          doc["errorPath"] = errorPath;
          doc["jsonError"] = jsonError;
          doc["meterSerial"] = serialUnoString;

          serializeJson(doc, datos);
          Serial.println(datos);
          delay(5000);
          meterSerial = serialUnoString;
          sensorReadings = httpPOSTReq(server, serverLan, errorPath, jsonError, meterSerial);
          M5.lcd.print(sensorReadings);
          delay(5000);
        }
        if (address == "02") {
          meterSerial = serialDosString;
          sensorReadings = httpPOSTReq(server, serverLan, errorPath, jsonError, meterSerial);
          String datos = "";
          StaticJsonDocument<300> doc;
          doc["server"] = server;
          doc["serverLan"] = serverLan;
          doc["errorPath"] = errorPath;
          doc["jsonError"] = jsonError;
          doc["meterSerial"] = meterSerial;

          serializeJson(doc, datos);
          delay(1000);
          Serial.println(datos);
        }
        if (address == "03") {
          meterSerial = serialTresString;
          sensorReadings = httpPOSTReq(server, serverLan, errorPath, jsonError, meterSerial);
          String datos = "";
          StaticJsonDocument<300> doc;
          doc["server"] = server;
          doc["serverLan"] = serverLan;
          doc["errorPath"] = errorPath;
          doc["jsonError"] = jsonError;
          doc["meterSerial"] = meterSerial;

          serializeJson(doc, datos);
          delay(1000);
          Serial.println(datos);
        }
      }
    }
    delay(600);
    esp_task_wdt_reset();
  } while (numberqueries > 0);
  return;
}

String addLeadingZeros(String binaryNumber, int desiredLength) {
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

int convertir(String binStr) {
  int val = 0;
  for (int i = 0; i < binStr.length(); i++) {
    val = val * 2 + (binStr[i] - 48);
  }
  return val;
}
void DecodeData_modbus(String datos) {
  const int numValues = 8;  // Número de valores a extraer

  for (int i = 0; i < numValues; i++) {
    String valueStr = datos.substring(i * 2, i * 2 + 2);
    plots_Modbus[i] = strtoul(valueStr.c_str(), nullptr, 16);
  }
}

void enviardato_Modbus() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  Serial2.write(plots_Modbus, 8);
}

String serialCheck_Modbus() {
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

String hexToBin(String hexString) {
  unsigned int decValue = 0;
  int nextInt;
  for (int i = 0; i < hexString.length(); i++) {
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    decValue = (decValue * 16) + nextInt;
  }
  String NumberBinary = String(decValue, BIN);
  return NumberBinary;
}
