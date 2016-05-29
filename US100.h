#include <SoftwareSerial.h>
#include <Arduino.h>

#include "Sensor.h"

#include "log.h"

#ifndef US100_H
#define US100_H

class US100 {
  public:
    US100(int rxPin, int txPin);

    int getDistanceMillimeter();
    int getTempCelcius();

  private:
    int rxPin;
    int txPin;

    SoftwareSerial *softSerial;
};

class US100DistanceSensor : public Sensor {
  public:
    US100DistanceSensor(US100 *us100);

    String getName();
    String getValue();

  private:
    US100* us100;
};

class US100TemperatureSensor : public Sensor {
  public:
    US100TemperatureSensor(US100 *us100);

    String getName();
    String getValue();

  private:
    US100* us100;
};

#endif US100_H
