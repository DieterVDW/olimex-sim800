#include "US100.h"

US100::US100(int rxPin, int txPin) {
  this->rxPin = rxPin;
  this->txPin = txPin;

  this->softSerial = new SoftwareSerial(rxPin, txPin);

  softSerial->begin(9600);
}

int US100::getDistanceMillimeter() {
  softSerial->write(0x55);
  while (softSerial->available() == 0) {
    delay(100);
  }
  byte high = softSerial->read();
  byte low = softSerial->read();

  int millimeter = high * 256 + low;
  LOG("US100: Read distance in mm: " + String(millimeter));
  return millimeter;
}

int US100::getTempCelcius() {
  softSerial->write(0x50);
  while (softSerial->available() == 0) {
    delay(100);
  }
  byte temp = softSerial->read();
  int tempCelcius = temp - 45;

  LOG("US100: Read temperature in celcius: " + String(tempCelcius));
  return tempCelcius;
};


//// Distance sensor

US100DistanceSensor::US100DistanceSensor(US100* us100) {
  this->us100 = us100;
}

String US100DistanceSensor::getName() {
  return "us100-distance";
}

String US100DistanceSensor::getValue() {
  return String(us100->getDistanceMillimeter());
}

//// Temperature sensor

US100TemperatureSensor::US100TemperatureSensor(US100* us100) {
  this->us100 = us100;
}

String US100TemperatureSensor::getName() {
  return "us100-temp";
}

String US100TemperatureSensor::getValue() {
  return String(us100->getTempCelcius());
}

