#ifndef CRC_CALCULATOR_H
#define CRC_CALCULATOR_H

#include <Arduino.h>

class CrcCalculator {
private:
  byte StrtoByte(String str_value) {
    char char_buff[3];
    str_value.toCharArray(char_buff, 3);
    byte byte_value = strtoul(char_buff, NULL, 16);
    return byte_value;
  }
public:
  String cyclic_redundancy_check(String raw_msg_data) {
    byte raw_msg_data_byte[raw_msg_data.length() / 2];

    for (int i = 0; i < raw_msg_data.length() / 2; i++) {
      raw_msg_data_byte[i] = StrtoByte(raw_msg_data.substring(2 * i, 2 * i + 2));
    }

    uint16_t crc = 0xFFFF;
    String crc_string = "";

    for (int pos = 0; pos < raw_msg_data.length() / 2; pos++) {
      crc ^= (uint16_t)raw_msg_data_byte[pos];
      for (int i = 8; i != 0; i--) {
        if ((crc & 0x0001) != 0) {
          crc >>= 1;
          crc ^= 0xA001;
        } else
          crc >>= 1;
      }
    }


    crc_string = String(crc, HEX);
    crc_string.toUpperCase();


    if (crc_string.length() == 1) {
      crc_string = "000" + crc_string;
    } else if (crc_string.length() == 2) {
      crc_string = "00" + crc_string;
    } else if (crc_string.length() == 3) {
      crc_string = "0" + crc_string;
    } else {
      //OK
    }

    crc_string = crc_string.substring(2, 4) + crc_string.substring(0, 2);

    return crc_string;
  }
};

#endif
