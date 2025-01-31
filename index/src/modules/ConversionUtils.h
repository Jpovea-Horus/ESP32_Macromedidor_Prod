#ifndef ConversionUtils_H
#define ConversionUtils_H

class ConversionUtils {
public:
  String dec_to_hex(int dec_value, byte desiredStringLength) {
    String hexString = String(dec_value, HEX);
    while (hexString.length() < desiredStringLength) hexString = "0" + hexString;
    return hexString;
  }

  int hex_to_dec(String hex_str) {
    unsigned int decValue = 0;
    int nextInt;
    for (int i = 0; i < hex_str.length(); i++) {
      nextInt = int(hex_str.charAt(i));
      if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
      if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
      if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
      nextInt = constrain(nextInt, 0, 15);
      decValue = (decValue * 16) + nextInt;
    }
    return decValue;
  }

  char* string_to_char_array(String str) {
    int datoString_len = str.length() + 1;
    char* charArray = new char[datoString_len];
    str.toCharArray(charArray, datoString_len);
    return charArray;
  }

  String string_to_mac(const char* str, uint8_t* mac) {
    char buffer[20];
    strcpy(buffer, str);
    const char* delims = ":";
    char* tok = strtok(buffer, delims);
    for (int i = 5; i >= 0; i--) {
      mac[i] = strtol(tok, NULL, 16);
      tok = strtok(NULL, delims);
    }
    return String(str);
  }

  String hex_to_bin(String hexString) {
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
};

#endif