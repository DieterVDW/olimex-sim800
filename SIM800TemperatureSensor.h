#include "Sensor.h"
#include "SIM800.h"

#ifndef SIM800TEMP_H
#define SIM800TEMP_H

class SIM800TemperatureSensor: public Sensor {
  public:
    SIM800TemperatureSensor(SIM800* sim800);

    String getName();
    String getValue();

  private:
    SIM800* sim800;
};

#endif
