#include "esp_bit_defs.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/def.h"
#include "lwip/ip_addr.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "esp32-hal.h"
#include "esp32-hal-log.h"
#include "esp_http_client.h"
#include "esp_err.h"

#include <netdb.h>
#include "esp_eth.h"
#include "esp_ping.h"
#include "ping/ping_sock.h"

#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "../../config/EthernetConfig.h"

#ifndef NETWORK_HANDLER_H
#define NETWORK_HANDLER_H


class NetworkHandler {
private:

  esp_http_client_config_t http_config;
  char base_url[100];
  char lan_url[100];
  char api_key[50];
  bool is_wifi_connected;

  esp_http_client_handle_t create_http_client(const char* path, const char* serial_number_meter, esp_http_client_method_t method) {
    const char* selected_url = is_wifi_connected ? base_url : base_url;

    String full_url = build_full_url(selected_url, path, serial_number_meter);

    char url_buffer[200];
    strncpy(url_buffer, full_url.c_str(), sizeof(url_buffer) - 1);
    url_buffer[sizeof(url_buffer) - 1] = '\0';

    http_config.skip_cert_common_name_check = false;
    http_config.disable_auto_redirect = false;

    http_config.url = url_buffer;
    http_config.method = method;
    http_config.transport_type = HTTP_TRANSPORT_OVER_SSL;

    Serial.println("Configuración de cliente HTTP:");
    Serial.printf("URL: %s\n", http_config.url);
    Serial.printf("Método: %d\n", method);

    esp_http_client_handle_t client = esp_http_client_init(&http_config);

    if (client == NULL) {
      Serial.println("Error: No se pudo inicializar el cliente HTTP");
      return NULL;
    }

    esp_http_client_set_header(client, "Content-type", "application/json");
    esp_http_client_set_header(client, "Authorization", (String("Bearer") + String(api_key)).c_str());

    return client;
  }

  String build_full_url(const char* base_path, const char* path, const char* serial_number_meter) {
    String full_url = String(base_path) + String(path) + String(serial_number_meter);
    return full_url;
  }

public:

  void set_connection_type(bool wifi_connected) {
    is_wifi_connected = wifi_connected;
  }

  void init(const char* base_url, const char* lan_url, const char* api_key) {

    strncpy(this->base_url, base_url, sizeof(this->base_url) - 1);
    strncpy(this->lan_url, lan_url, sizeof(this->lan_url) - 1);
    strncpy(this->api_key, api_key, sizeof(this->api_key) - 1);

    http_config.url = "";
    http_config.is_async = true;
    http_config.cert_pem = root_cert_pem;
    http_config.method = HTTP_METHOD_GET;
    http_config.buffer_size = 1024;
    http_config.buffer_size_tx = 1024;
    http_config.timeout_ms = 5000;
  }

  String http_post_req(const char* path, const char* data, const char* serial_number_meter) {
    esp_http_client_handle_t client = create_http_client(path, serial_number_meter, HTTP_METHOD_POST);
    esp_err_t err;

    String response = "";

    int content_length = strlen(data);
    int retry_count = 0;

    do {
      err = esp_http_client_open(client, content_length);
      if (err != ESP_OK) {
        Serial.printf("Error abriendo conexión (intento %d): %s\n",
                      retry_count, esp_err_to_name(err));
        // esp_http_client_cleanup(client);
        delay(1000);
      }
    } while (err != ESP_OK && retry_count++ < 3);

    if (err != ESP_OK) {
      Serial.println("No se pudo abrir conexión después de varios intentos");
      esp_http_client_cleanup(client);
      return response;
    }

    int write_bytes = esp_http_client_write(client, data, content_length);
    if (write_bytes != content_length) {
      Serial.printf("Error writing data. Wrote %d of %d bytes\n", write_bytes, content_length);
      esp_http_client_cleanup(client);
      return response;
    }

    Serial.printf("write_bytes: %d (de %d)\n", write_bytes, content_length);

    int headers_len = esp_http_client_fetch_headers(client);
    if (headers_len < 0) {
      Serial.printf("HTTP_CLIENT Error fetching headers: %s\n", esp_err_to_name(headers_len));
    } else {
      Serial.printf("Headers fetched: %d bytes\n", headers_len);
    }

    int status_code = esp_http_client_get_status_code(client);
    Serial.printf("Status Code: %d\n", status_code);

    int content_length_response = esp_http_client_get_content_length(client);
    Serial.printf("Response Content Length: %d\n", content_length_response);

    char* buffer = (char*)malloc(content_length_response + 1);

    if (buffer) {
      int read_bytes = esp_http_client_read(client, buffer, content_length_response);

      if (read_bytes > 0) {
        buffer[read_bytes] = '\0';  // Null-terminate
        response = String(buffer);

        Serial.println("Respuesta recibida:");
        Serial.println(response);
      } else {
        Serial.println("No se recibieron datos");
      }
      free(buffer);
    }

    esp_http_client_close(client);

    esp_http_client_cleanup(client);
    return response;
  }

  String http_get_req(const char* path, const char* serial_number_meter) {
    esp_http_client_handle_t client = create_http_client(path, serial_number_meter, HTTP_METHOD_GET);
    esp_err_t err;
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
      return "";
    }

    int headers_len = esp_http_client_fetch_headers(client);
    if (headers_len < 0) {
      Serial.printf("HTTP_CLIENT Error fetching headers: %d\n", headers_len);
    }

    esp_http_client_set_method(client, HTTP_METHOD_GET);
    bool status_data = esp_http_client_is_complete_data_received(client);
    String response = "";

    if (err == ESP_OK) {
      Serial.println("Conexión exitosa al servidor");
      int content_length = esp_http_client_get_content_length(client);
      if (content_length > 0) {
        char* buffer = (char*)malloc(content_length + 1);
        if (buffer) {
          int read_bytes = esp_http_client_read_response(
            client,
            buffer,
            content_length);

          if (read_bytes > 0) {
            buffer[read_bytes] = '\0';
            Serial.printf("Leídos: %d bytes\n", read_bytes);
            response = String(buffer);
            Serial.println(buffer);
          } else {
            Serial.println("No se pudieron leer los datos");
          }

          free(buffer);
        } else {
          Serial.println("Error al leer el cuerpo. read_len es 0.");
        }
      }

      int status_code = esp_http_client_get_status_code(client);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Status Code: " + String(status_code));
      Serial.println("Status Code: " + String(status_code));
    } else {
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Error in request");
    }

    esp_http_client_close(client);

    esp_http_client_cleanup(client);
    return response;
  }
};

#endif