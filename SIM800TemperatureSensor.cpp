#include "SIM800TemperatureSensor.h"

SIM800TemperatureSensor::SIM800TemperatureSensor(SIM800* sim800) {
  this->sim800 = sim800;
}

String SIM800TemperatureSensor::getName() {
  return "sim800-temp";
}

String SIM800TemperatureSensor::getValue() {
  String response = sim800->sendStringForOK("AT+CMTE?");

  // Parse temperature from response
  int commaPos = response.indexOf(',');
  int okPos = response.indexOf('OK', commaPos);
  String value = response.substring(commaPos + 1, okPos-1);
  LOG("SIM800: Temperature value read: " + value);
  return value;
}

