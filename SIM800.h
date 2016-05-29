#include <Arduino.h>

#include "log.h"

#ifndef SIM800_H
#define SIM800_H

#define DEFAULT_PIN 6776
#define DEFAULT_GSM_POWERSWITCH_PIN 11

class SIM800 {
  public:
    SIM800(int powerPin = DEFAULT_GSM_POWERSWITCH_PIN, int pinCode = DEFAULT_PIN);
    
    int testActive();
    void turnOn();
    void turnOff();

    void echoOff();
    void checkPIN();

    void enableGPRS(String accessPoint);
    int startTCPConnection(String server, int port);
    void stopTCPConnection();

    String sendStringForOK(String s);
    void sendString(String s);
    String readString();
    String readString(int maxWaitTime);

  private:
    int powerPin;
    int pinCode;
};

#endif

