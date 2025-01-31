#ifndef MODBUS_VARIABLE_CONVERTER_H
#define MODBUS_VARIABLE_CONVERTER_H

#include "../../lib/converters.h"

class ModbusVariableConverter {
  private:
    String name_var;

  public:
    ModbusVariableConverter() {}

    String get_name_var() {
      return name_var;
    }

    void convert_modbus_variable_name(String name_v) {
      String sub_name_v = name_v.substring(4, 8);
      int type_case = hexToDec(sub_name_v);

      switch (type_case) {
        case 0:    name_var = "VFA";      break;
        case 2:    name_var = "VFB";      break;
        case 4:    name_var = "VFC";      break;
        case 6:    name_var = "CFA";      break;
        case 8:    name_var = "CFB";      break;
        case 10:   name_var = "CFC";      break;
        case 12:   name_var = "PAFA";     break;
        case 14:   name_var = "PAFB";     break;
        case 16:   name_var = "PAFC";     break;
        case 30:   name_var = "FPFA";     break;
        case 32:   name_var = "FPFB";     break;
        case 34:   name_var = "FPFC";     break;
        case 52:   name_var = "TSE";      break;
        case 56:   name_var = "TSVa";     break;
        case 60:   name_var = "TSVar";    break;
        case 62:   name_var = "TFT";      break;
        case 70:   name_var = "FHz";      break;
        case 72:   name_var = "TImKwh";   break;
        case 74:   name_var = "TExKwh";   break;
        case 76:   name_var = "TImKVarh"; break;
        case 78:   name_var = "TExKVarh"; break;
        case 80:   name_var = "TVah";     break;
        case 82:   name_var = "Ah";       break;
        case 200:  name_var = "VFAFB";    break;
        case 202:  name_var = "VFBFC";    break;
        case 204:  name_var = "VFCFA";    break;
        case 224:  name_var = "NC";       break;
        case 234:  name_var = "FALNVTHD"; break;
        case 236:  name_var = "FBLNVTHD"; break;
        case 238:  name_var = "FCLNVTHD"; break;
        case 240:  name_var = "FACTHD";   break;
        case 242:  name_var = "FBCTHD";   break;
        case 244:  name_var = "FCCTHD";   break;
        case 342:  name_var = "TKWh";     break;
      }
    }
};

#endif
