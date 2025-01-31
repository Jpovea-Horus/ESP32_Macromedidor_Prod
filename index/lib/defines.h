#include "Arduino.h"
#include "WString.h"
#ifndef defines_h
#define defines_h

// ----------- CONSTANTS AND DEFINITIONS -----------
#define SIZE 250

#define SCK_PIN 18
#define MISO_PIN 19
#define MOSI_PIN 23
#define CS_PIN 26
#define INT_PIN 34
#define RST 13


#define MAX_METERS 5
#define USE_INTERNAL_PIN_LOOPBACK 2
#define BAUD 9600
#define TEST_UART 2
#define RXPIN 16
#define TXPIN 17
#define DE 32
#define RE 33
#define PORT 80
#define STORAGE "storage"

//-------------env_memori-----
nvs_handle_t handle;

//-------------configuration variables----------

struct meter_communicator_config {
  String registered = "false";
  String project_id = "";
  String communicator_id = "";
  String operation_mode = "";
  String meter_serials[MAX_METERS];
  String meter_addresses[MAX_METERS];
  String num_meters_to_install = "";
  String network_ssid = "";
  String network_password = "";
  String mac_by_string = "";
  int status = 0;
};

// ----------- NETWORK CONFIGURATION -----------
const char* ssid = "";      // Enter SSID
const char* password = "";  // Enter Password
const char* api_key = "TOKEN";
const char* host = "esp32";

const char* ntp_server_1 = "pool.ntp.org";
const char* ntp_server_2 = "time.nist.gov";
const long gmt_offset_sec = 3600;
const int daylight_offset_sec = 3600;

// ----------- SERVER CONFIGURATION -----------
const String server_pro = "https://www.macrotest.horussmartenergyapp.com";
const String server_lan = "www.macrotest.horussmartenergyapp.com";
const String server_local = "http://10.0.5.107:2318";

// Rutas de la API
String path = "/api/v1/meterhistory/plots/";
String path_communicator = "/api/v1/Communicator/";
String path_registered = "/api/v1/meter/";
String error_path = "/api/v1/meterhistory/errorMeter/";
String status_serial_path = "/api/v1/meter/serial/";


// ----------- CONFIGURATION AND STATE VARIABLES -----------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 10, 10, 1, 3 };

String ssid_string = "";
String password_string = "";
String dir_string = "";
String meter_serial = "";
String serial_one_string = "";
String serial_two_string = "";
String serial_three_string = "";
String mac_by_string = "";
String mode = "";
String dir = "";
String registered_communicator = "false";
String plots[16] = { "", "", "", "", "", "" };
String number_meters[4];

// ----------- TEMPORARY VARIABLES -----------
String time_string = "";
String minutes = "";
String x = "0";

// ----------- METER VARIABLES -----------
String meter_one_modbus = "";
String meter_two_modbus = "";
String meter_three_modbus = "";
int meter_count = 0;

// ----------- GLOBAL VARIABLES -----------
int conteo = 0;
int project_id = 0;
int status = 0;
int id_communicator = 0;
int n_elements = 0;
int lan = 0;
int http_response_code = 0;

// ----------- UI COORDINATES -----------
int axis_x = 110;
int axis_y = 100;

int nElements = 0;
int LAN = 0;

const char* str = "DE:AD:BE:EF:FE:ED";

const char* root_cert_pem = "-----BEGIN CERTIFICATE-----\n"
                            "MIIEkjCCA3qgAwIBAgITBn+USionzfP6wq4rAfkI7rnExjANBgkqhkiG9w0BAQsF\n"
                            "ADCBmDELMAkGA1UEBhMCVVMxEDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNj\n"
                            "b3R0c2RhbGUxJTAjBgNVBAoTHFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4x\n"
                            "OzA5BgNVBAMTMlN0YXJmaWVsZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1\n"
                            "dGhvcml0eSAtIEcyMB4XDTE1MDUyNTEyMDAwMFoXDTM3MTIzMTAxMDAwMFowOTEL\n"
                            "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
                            "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
                            "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
                            "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
                            "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
                            "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
                            "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
                            "jgSubJrIqg0CAwEAAaOCATEwggEtMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/\n"
                            "BAQDAgGGMB0GA1UdDgQWBBSEGMyFNOy8DJSULghZnMeyEE4KCDAfBgNVHSMEGDAW\n"
                            "gBScXwDfqgHXMCs4iKK4bUqc8hGRgzB4BggrBgEFBQcBAQRsMGowLgYIKwYBBQUH\n"
                            "MAGGImh0dHA6Ly9vY3NwLnJvb3RnMi5hbWF6b250cnVzdC5jb20wOAYIKwYBBQUH\n"
                            "MAKGLGh0dHA6Ly9jcnQucm9vdGcyLmFtYXpvbnRydXN0LmNvbS9yb290ZzIuY2Vy\n"
                            "MD0GA1UdHwQ2MDQwMqAwoC6GLGh0dHA6Ly9jcmwucm9vdGcyLmFtYXpvbnRydXN0\n"
                            "LmNvbS9yb290ZzIuY3JsMBEGA1UdIAQKMAgwBgYEVR0gADANBgkqhkiG9w0BAQsF\n"
                            "AAOCAQEAYjdCXLwQtT6LLOkMm2xF4gcAevnFWAu5CIw+7bMlPLVvUOTNNWqnkzSW\n"
                            "MiGpSESrnO09tKpzbeR/FoCJbM8oAxiDR3mjEH4wW6w7sGDgd9QIpuEdfF7Au/ma\n"
                            "eyKdpwAJfqxGF4PcnCZXmTA5YpaP7dreqsXMGz7KQ2hsVxa81Q4gLv7/wmpdLqBK\n"
                            "bRRYh5TmOTFffHPLkIhqhBGWJ6bt2YFGpn6jcgAKUj6DiAdjd4lpFw85hdKrCEVN\n"
                            "0FE6/V1dN2RMfjCyVSRCnTawXZwXgWHxyvkQAiSr6w10kY17RSlQOYiypok1JR4U\n"
                            "akcjMS9cmvqtmg5iUaQqqcT5NJ0hGA==\n"
                            "-----END CERTIFICATE-----\n";

#endif  //defines_h
