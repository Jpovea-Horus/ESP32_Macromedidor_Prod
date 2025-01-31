#include "esp_err.h"
#include "esp32-hal.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "WString.h"
#include "HardwareSerial.h"
#ifndef writeAndReadEEPROM_h
#define writeAndReadEEPROM_h

#define RW_MODE false
#define RO_MODE true

class MemoryHandler {
private:
  void handle_nvs_error(esp_err_t err) {
    switch (err) {
      case ESP_ERR_NVS_INVALID_HANDLE:
        Serial.println("Error: Handle inválido o cerrado");
        break;
      case ESP_ERR_NVS_READ_ONLY:
        Serial.println("Error: Almacenamiento abierto solo para lectura");
        break;
      case ESP_ERR_NVS_INVALID_NAME:
        Serial.println("Error: Nombre de clave inválido");
        break;
      case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
        Serial.println("Error: No hay suficiente espacio en el almacenamiento");
        break;
      case ESP_ERR_NVS_REMOVE_FAILED:
        Serial.println("Error: No se pudo actualizar el valor. Operación de escritura en flash falló");
        break;
      case ESP_ERR_NVS_VALUE_TOO_LONG:
        Serial.println("Error: Valor de cadena demasiado largo");
        break;
      default:
        Serial.print("Error desconocido: ");
        Serial.println(err);
        break;
    }
  }
public:
  char* get_data(nvs_handle_t handle, const char* storage, const char* key) {
    nvs_open(storage, NVS_READONLY, &handle);
    esp_err_t err;
    size_t required_size;

    err = nvs_get_str(handle, key, NULL, &required_size);
    if (err != ESP_OK) {
      M5.Lcd.printf("no data found: %s\n", key);
      delay(2000);
      return strdup("404");
    }

    char* data = (char*)malloc(required_size);
    if (data == NULL) {
      Serial.println("Error de asignación de memoria");
      return NULL;
    }

    err = nvs_get_str(handle, key, data, &required_size);
    if (err != ESP_OK) {
      Serial.print("Error al leer datos: ");
      Serial.println(err);
      free(data);
      return NULL;
    }

    for (size_t i = 0; i < required_size; i++) {
      if (!isPrintable(data[i]) && data[i] != '\0') {
        Serial.println("Datos no imprimibles");
        free(data);
        return NULL;
      }
    }
    return data;
  }

  void set_data(nvs_handle_t handle, const char* storage, const char* key, const char* value) {
    esp_err_t err = nvs_open(storage, NVS_READWRITE, &handle);

    if (err != ESP_OK) {
      Serial.println("error opening flash memory");
      handle_nvs_error(err);
      return;
    }

    Serial.printf("key: %s, value %s \n", key, value);
    err = nvs_set_str(handle, key, value);
    if (err != ESP_OK) {
      Serial.println("error occurred while saving the value");
      handle_nvs_error(err);
      return;
    }
    nvs_commit(handle);
    nvs_close(handle);

    Serial.println("dato guardado");
  }

  void check_nvs_memory(nvs_handle_t handle, const char* storage, const char* key) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      Serial.println("Detected new NVS version.");

      nvs_flash_erase();
      nvs_flash_init();
    }
    err = nvs_open(storage, NVS_READONLY, &handle);
    if (err != ESP_OK) {
      Serial.println("Error opening NVS");
      return;
    }
    nvs_stats_t nvs_stats;
    nvs_get_stats(NULL, &nvs_stats);
    Serial.printf("Count: UsedEntries = (%lu), FreeEntries = (%lu)\n",
                  nvs_stats.used_entries, nvs_stats.free_entries);

    nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY);
    if (!it) {
      Serial.println("No entries found in NVS.");
      nvs_close(handle);
      return;
    }
    while (it != NULL) {
      nvs_entry_info_t info;
      nvs_entry_info(it, &info);

      Serial.print("Key matched: ");
      Serial.println(info.key);
      it = nvs_entry_next(it);
    }
    nvs_release_iterator(it);
    nvs_close(handle);
    return;
  }

  void delete_nvs_memory(nvs_handle_t handle, const char* storage, const char* key) {
    esp_err_t err = nvs_open(storage, NVS_READWRITE, &handle);

    if (err != ESP_OK) {
      Serial.println("an error occurred");
      Serial.println(err);
      return;
    }
    err = nvs_erase_key(handle, key);
    if (err != ESP_OK) {
      Serial.println("has not been deleted correctly ");
      return;
    }
    nvs_commit(handle);
    Serial.println("successfully deleted ");
  }
};


#endif