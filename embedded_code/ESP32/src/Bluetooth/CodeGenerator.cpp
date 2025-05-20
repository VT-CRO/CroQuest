// src/Bluetooth/CodeGenerator.cpp
#include "CodeGenerator.hpp"

String generateRandomCode() {
  String code = "";
  for (int i = 0; i < 6; i++) {
    code += String(random(0, 10));
  }
  return code;
}
