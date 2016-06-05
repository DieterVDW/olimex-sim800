#include <Arduino.h>

#include "log.h"

#ifndef SIM800_H
#define SIM800_H

class SIM800 {
  public:
    SIM800(int powerPin, int pinCode);
    
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
    void send(String s);
    void write(char c);
    void flush();
    String readString();
    String readString(int maxWaitTime);

  private:
    int powerPin;
    int pinCode;
};

#endif

