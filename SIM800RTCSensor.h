#include "Sensor.h"
#include "SIM800.h"

#ifndef SIM800_RTC_H
#define SIM800_RTC_H

class SIM800RTCSensor : public Sensor {
  public:
    SIM800RTCSensor(SIM800* sim800);

    String getName();
    String getValue();

  private:
    SIM800* sim800;
};

#endif SIM800_RTC_H
