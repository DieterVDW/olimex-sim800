#include "SIM800RTCSensor.h"

SIM800RTCSensor::SIM800RTCSensor(SIM800* sim800) {
  this->sim800 = sim800;
}

String SIM800RTCSensor::getName() {
  return "sim800-rtc";
}

String SIM800RTCSensor::getValue() {
  String response = sim800->sendStringForOK("AT+CCLK?");

  // Parse temperature from response
  int spacePos = response.indexOf(' ');
  int okPos = response.indexOf('OK', spacePos);
  String value = response.substring(spacePos + 2, okPos - 2);
  LOG("SIM800: RTC value read: " + value);
  return value;
}



