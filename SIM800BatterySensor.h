#include "Sensor.h"
#include "SIM800.h"

#ifndef SIM800_BATTERY_H
#define SIM800_BATTERY_H

class SIM800BatterySensor : public Sensor {
  public:
    SIM800BatterySensor(SIM800* sim800);

    String getName();
    String getValue();

  private:
    SIM800* sim800;
};

#endif SIM800_BATTERY_H
