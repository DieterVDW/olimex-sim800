#include "SIM800BatterySensor.h"

SIM800BatterySensor::SIM800BatterySensor(SIM800* sim800) {
  this->sim800 = sim800;
}

String SIM800BatterySensor::getName() {
  return "sim800-battery";
}

String SIM800BatterySensor::getValue() {
  String response = sim800->sendStringForOK("AT+CBC");

  // Parse temperature from response
  int commaPos = response.indexOf(',');
  commaPos = response.indexOf(',', commaPos+1);
  int okPos = response.indexOf('OK', commaPos);
  String value = response.substring(commaPos + 1, okPos-1);
  LOG("SIM800: Battery value read: " + value);
  return value;
}



