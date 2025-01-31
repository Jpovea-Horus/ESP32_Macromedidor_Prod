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
#include <SPI.h>
#include <EthernetLarge.h>
#include <EthernetUdp.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#include "EEPROM.h"
#include "lib/defines.h"
#include "image/xbm.h"
#include "image/loginIndex.h"
#include "image/updateIndex.h"
#include "lib/writeAndReadEEPROM.h"
#include "CronAlarms.h"

TaskHandle_t Task1;
CronId id;

JSONVar myArrayPruebab3, myArrayPruebab4, myArrayPruebab2;
JSONVar myArray_Modbus;
JSONVar myArrayPrueba;

int parity = 1;
int initialsize = 0;

WiFiClientSecure client0;
EthernetClient client1;

WiFiUDP ntpUDP;
EthernetUDP Udp;

NTPClient timeClient1(Udp, "0.pool.ntp.org", 36000, 60000);
NTPClient timeClient0(ntpUDP, "0.pool.ntp.org", 36000, 60000);

WebServer serverUpdate(80);

// volatile size_t sent_bytes = 0, received_byte = 0;
// unsigned long int epochTimeFromServer = 0;
unsigned long previousMillis = 0;
const unsigned long interval = 1000;
unsigned long previousMillis_test = 0;
const unsigned long interval_test = 60000;
time_t epoch;

void increaseWatchdogTime() {
  esp_task_wdt_init(10, true);  // Aumentar el tiempo del watchdog a 10 segundos
}

void queryMeter() {
  M5.lcd.clear();
  M5.Lcd.setCursor(0, 0);
  int numeroMeter = readStringFromEEPROM(20).toInt();
  // if (registeredCommunicator == "true") {
  //   M5.Lcd.print("estado de registro: ");
  //   M5.Lcd.println(registeredCommunicator);
  //   delay(2000);
  if (numeroMeter > 1) {
    for (int i = 0; i < numeroMeter; i++) {
      Serial.print(numberMeters[i]);
      Serial.print(" ");
      Serial.print(i);
      Serial.print(" ");
      ConsultaMedidorModbus(numberMeters[i]);
      delay(100);
    }
  } else {
    ConsultaMedidorModbus(numberMeters[0]);
  }
  // } else {
  //   M5.Lcd.print("estado de registro: ");
  //   M5.Lcd.println(registeredCommunicator);
  //   delay(1000);
  //   configureRed();
  //   M5.Lcd.println("enviando registro....");
  //   if (idCommunicator != 0) {
  //     bool statusProjectMeter;
  //     if (numberMeters[0] == "01") {
  //       if (queryDBSerial(serialUnoString)) {
  //         String response = sendRegiterdMeter(serialUnoString);
  //         M5.Lcd.println(response);
  //         statusProjectMeter = true;
  //       } else {
  //         M5.Lcd.println("medidor no habilitado");
  //         M5.Lcd.println(serialUnoString);
  //         statusProjectMeter = false;
  //       }
  //     }
  //     if (numberMeters[1] == "02") {
  //       if (queryDBSerial(serialDosString)) {
  //         sendRegiterdMeter(serialDosString);
  //       } else {
  //         M5.Lcd.print("medidor no habilitado: ");
  //         M5.Lcd.println(serialDosString);
  //       }
  //     }
  //     if (statusProjectMeter == true) {
  //       registeredCommunicator = "true";
  //       writeStringToEEPROM(200, registeredCommunicator);
  //     }
  //     delay(3000);
  //   }
  // }
}

void setup() {
  M5.begin();
  M5.Power.begin();
  while (!Serial) {
    ;
  }
  M5.lcd.setTextSize(2);
  delay(1000);
  if (parity == 1) {
    Serial2.begin(9600);
  } else {
    Serial2.begin(BAUD, SERIAL_8E1, RXPIN, TXPIN);
  }
  M5.lcd.clear();
  M5.Lcd.setCursor(0, 0);
  if (!EEPROM.begin(SIZE)) {
    M5.Lcd.println("\nFailed to initialise EEPROM!");
    delay(10000);
  }
  M5.Lcd.println("\nRead data from EEPROM. Values are:");

  if (EEPROM.read(249) == 255) {
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0);
      EEPROM.commit();
    }
  } else {
    for (int i = 0; i < SIZE; i++) {
      M5.Lcd.printf("%d ", EEPROM.read(i));
    }
  }
  delay(5000);

  if (EEPROM.read(200) > 0) {
    registeredCommunicator = readStringFromEEPROM(200);
    M5.Lcd.println(registeredCommunicator);
  } else {
    registeredCommunicator = "false";
  }

  if (registeredCommunicator == "true" || EEPROM.read(0) > 0) {
    M5.Lcd.fillScreen(0);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("estado de registro: ");
    M5.Lcd.println(registeredCommunicator);
    delay(1500);

    if (EEPROM.read(0) > 0) {
      projectId = readStringFromEEPROM(0).toInt();
      M5.Lcd.print("codigo de projecto: ");
      M5.Lcd.println(projectId);
    }
    delay(1500);

    if (EEPROM.read(123) > 0) {
      idCommunicator = readStringFromEEPROM(123).toInt();
      M5.Lcd.print("Communicator: ");
      M5.Lcd.println(idCommunicator);
      Serial.println(idCommunicator);
    } else {
      M5.Lcd.print("Communicator: ");
      M5.Lcd.println(idCommunicator);
    }
    delay(1500);

    if (EEPROM.read(120) > 0) {
      modo = readStringFromEEPROM(120);
      M5.Lcd.print("modo de operacion: ");
      M5.Lcd.println(modo);
      M5.Lcd.println(modo);
    }
    delay(1500);
    Serial.println(EEPROM.read(20));
    if (EEPROM.read(20) > 0) {
      String dato = readStringFromEEPROM(20);
      if (dato == "3") {
        Serial.print("dato: ");
        Serial.println(dato);

        numberMeters[0] = "01";
        numberMeters[1] = "02";
        numberMeters[2] = "03";

        Serial.print("adress: ");
        Serial.println(numberMeters[0]);
        Serial.println(numberMeters[1]);
        Serial.println(numberMeters[2]);

      } else if (dato == "2") {
        numberMeters[0] = "01";
        numberMeters[1] = "02";
        M5.Lcd.print("adress: ");
        M5.Lcd.println(numberMeters[0]);
        M5.Lcd.println(numberMeters[1]);
      } else {
        numberMeters[0] = "01";
        M5.Lcd.print("adress: ");
        M5.Lcd.println(numberMeters[0]);
      }
      Dirstring = readStringFromEEPROM(20);
    }
    delay(1500);

    if (EEPROM.read(30) > 0) {
      serialUnoString = readStringFromEEPROM(30);
      Serial.println(serialUnoString);
      StaticJsonDocument<300> seriales;
      String SerialesMedidor = "";
      M5.Lcd.begin();
      seriales["medidorUno"] = serialUnoString;
      if (EEPROM.read(40) > 0) {
        serialDosString = readStringFromEEPROM(40);
        seriales["medidorDos"] = serialDosString;
      }
      if (EEPROM.read(50) > 0) {
        serialTresString = readStringFromEEPROM(50);
        seriales["medidorTres"] = serialTresString;
      }
      serializeJson(seriales, SerialesMedidor);
      M5.Lcd.print("Serial de Medidores: ");
      M5.Lcd.println(SerialesMedidor);
      Serial.println(SerialesMedidor);
    }
    delay(1500);

    if (modo == "1") {
      if (EEPROM.read(70) > 0) {
        ssidString = readStringFromEEPROM(70);
        M5.Lcd.println(ssidString);
        initialsize = EEPROM.read(70);
        if (EEPROM.read(90) > 0) {
          passwordString = readStringFromEEPROM(90);
          M5.Lcd.println(passwordString);
          initialsize = EEPROM.read(90);
        }
      }
    } else {
      if (EEPROM.read(130) > 0) {
        macbystring = readStringFromEEPROM(130);
        Serial.print(macbystring);
        M5.Lcd.print("MAC: ");
        M5.Lcd.println(macbystring);
      }
    }
    delay(1500);
    configureRed();

    if (registeredCommunicator == "false") {
      M5.lcd.clear();
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("comunicador no registrado");
      delay(1000);
      M5.Lcd.println("enviando registro....");
      delay(10000);
      if (idCommunicator == 0) {
        bool statusProjectMeter;
        if (numberMeters[0] == "01") {
          statusProjectMeter = true;
          // Serial.println(idCommunicator);
          // if (queryDBSerial(serialUnoString)) {
          //   String response = sendRegiterdMeter(serialUnoString);
          //   M5.Lcd.println(response);
          //   statusProjectMeter = true;
          // } else {
          M5.Lcd.println("medidor no habilitado");
          M5.Lcd.println(serialUnoString);
          // statusProjectMeter = false;
          // }
        }
        if (numberMeters[1] == "02") {
          // if (queryDBSerial(serialDosString)) {
          //   sendRegiterdMeter(serialDosString);
          // } else {
          M5.Lcd.print("medidor no habilitado: ");
          M5.Lcd.println(serialDosString);
          // }
        }
        if (numberMeters[2] == "03") {
          // if (queryDBSerial(serialDosString)) {
          //   sendRegiterdMeter(serialDosString);
          // } else {
          M5.Lcd.print("medidor no habilitado: ");
          M5.Lcd.println(serialTresString);
          // }
        }
        if (statusProjectMeter == true) {
          registeredCommunicator = "true";
          writeStringToEEPROM(200, registeredCommunicator);
        }
        delay(3000);
      }
    }
    delay(3000);

  } else if (registeredCommunicator == "false" && EEPROM.read(0) == 0) {
    M5.Lcd.setTextColor(WHITE);
    do {
      M5.lcd.clear();
      M5.Lcd.setCursor(0, 0);
      configureRed();
    } while (status > 0);
    
    M5.lcd.clear();
    M5.Lcd.println("comunicador no registrado");
    delay(3000);
    M5.Lcd.println("enviando registro....");
    // sendRegiterdCommunicator(serialUnoString);
    if (idCommunicator == 0) {
      M5.lcd.clear();
      M5.Lcd.setCursor(0, 0);
      bool statusProjectMeter;
      if (numberMeters[0] == "01") {
        // if (queryDBSerial(serialUnoString)) {
        // String response = sendRegiterdMeter(serialUnoString);
        // M5.Lcd.println(response);
        statusProjectMeter = true;
      } else {
        M5.Lcd.println("medidor no avilitado");
        M5.Lcd.println(serialUnoString);
      }
      if (numberMeters[1] == "02") {
        // if (queryDBSerial(serialDosString)) {
        // sendRegiterdMeter(serialDosString);
      } else {
        M5.Lcd.print("medidor no avilitado: ");
        M5.Lcd.println(serialDosString);
      }
      if (numberMeters[2] == "03") {
        // if (queryDBSerial(serialDosString)) {
        // sendRegiterdMeter(serialDosString);
      } else {
        M5.Lcd.print("medidor no avilitado: ");
        M5.Lcd.println(serialTresString);
      }
      if (statusProjectMeter == true) {
        registeredCommunicator = "true";
        writeStringToEEPROM(200, registeredCommunicator);
      }
    }
    delay(3000);
  }


  if (modo == "1") {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("cargando datos de time: ");
    timeClient0.begin();
    timeClient0.setTimeOffset(-18000);
    timeClient0.update();
    epoch = timeClient0.getEpochTime();
    Serial.println(epoch);
  } else {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("cargando datos de time: ");
    timeClient1.begin();
    timeClient1.setTimeOffset(-18000);
    timeClient1.update();
    epoch = timeClient1.getEpochTime();
    Serial.println(epoch);
  }

  struct tm *ptm = gmtime((time_t *)&epoch);
  timeval tv = { mktime(ptm), 0 };
  settimeofday(&tv, nullptr);
  Serial.print("Year: ");
  Serial.println(ptm->tm_year);

  if (ptm->tm_year <= 69) {
    // struct tm tm_newTime;
    // tm_newTime.tm_year = 2011 - 1900;
    // tm_newTime.tm_mon = 1 - 1;
    // tm_newTime.tm_mday = 1;
    // tm_newTime.tm_hour = 8;
    // tm_newTime.tm_min = 30;
    // tm_newTime.tm_sec = 0;
    // tm_newTime.tm_isdst = 0;

    // tv = { mktime(&tm_newTime), 0 };
    // settimeofday(&tv, nullptr);

    // time_t time_now = time(nullptr);
    // M5.Lcd.println(asctime(gmtime(&time_now)));
    M5.Lcd.println(gmtime((time_t *)&epoch));
    delay(30000);
    reset();
  } else {
    M5.Lcd.println(gmtime((time_t *)&epoch));
  }

  M5.Lcd.println("Ending setup...");
  delay(3000);

  if (parity == 1) {
    Cron.create("0 15 0 * * *", reset, false);
    Cron.create("0 */5 * * * *", queryMeter, false);
  }
  pinMode(DE, OUTPUT);
  pinMode(RE, OUTPUT);
  M5.Lcd.begin();
}

void loop() {
  M5.update();
  M5.Lcd.setTextColor(WHITE);
  serverUpdate.handleClient();

  if (M5.BtnA.isPressed()) {
    M5.lcd.clear();
    M5.lcd.setCursor(0, 0);
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0);
      EEPROM.commit();
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
#ifdef __AVR__
    system_tick();  // must be implemented at 1Hz
#endif
    time_t tnow = time(nullptr);
    // if (currentMillis - previousMillis_test >= interval_test) {`
    Time = String(asctime(gmtime(&tnow)));
    if (minutos != Time.substring(15, 16)) {
      minutos = Time.substring(15, 16);
      M5.Lcd.fillScreen(0);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Horus Smart Energi...");
      String fecha = Time.substring(0, 16);
      String year = Time.substring(20);
      fecha = fecha + " : " + year;
      M5.Lcd.drawString(fecha, 0, 60, 1);
      // previousMillis_test = currentMillis;
      if (modo == "1") {
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
        M5.Lcd.drawXBitmap(axisx, axisy, logo, logoWidth, logoHeight, TFT_WHITE, TFT_BLACK);
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
        M5.Lcd.drawXBitmap(axisx, axisy, logo, logoWidth, logoHeight, TFT_WHITE, TFT_BLACK);
      }
    }

    Cron.delay();
    previousMillis = currentMillis;
  }

  esp_task_wdt_reset();
}

void reset() {
  // bot.sendMessage(CHAT_ID, "reset system", "");
  ESP.restart();
}