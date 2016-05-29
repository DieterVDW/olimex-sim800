#include <Arduino.h>

#include "log.h"

#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  public:
    virtual String getName() = 0;
    virtual String getValue() = 0;
};

#endif

