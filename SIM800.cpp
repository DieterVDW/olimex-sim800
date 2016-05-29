#include "SIM800.h"

SIM800::SIM800(int powerPin, int pinCode) {
  this->powerPin = powerPin;
  this->pinCode = pinCode;
  
  // Power button pin: set low
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW); // LOW by default
}

int SIM800::testActive() {
  LOG("Testing GSM module presence ...");
  sendString("AT");
  delay(1000);
  if (Serial1.available() > 0) {
    String response = readString();
    if (response == "OK" || response == "ATOK") {
      return 1;
    }
  }
  return 0;
}

void SIM800::turnOn() {
  LOG("Trying to turn on GSM module!");
  digitalWrite(powerPin, HIGH);
  delay(1200);
  digitalWrite(powerPin, LOW); // High by default
}

void SIM800::turnOff() {
  LOG("Shutting down GSM module!");
  sendStringForOK("AT+CPOWD=1");
}

void SIM800::echoOff() {
  sendStringForOK("ATE0"); // Host echo off
}


void SIM800::checkPIN() {
  String response = sendStringForOK("AT+CPIN?");

  if (response.indexOf("READY") == -1) {
    LOG("Entering PIN");
    String command = "AT+CPIN=";
    response = sendStringForOK(command + pinCode);

    response = sendStringForOK("AT+CPIN?");
    if (response.indexOf("READY") == -1) {
      LOG("Could not verify PIN! Aborting!");
    }
  }

  LOG("PIN OK");
}

void SIM800::enableGPRS(String accessPoint) {
  LOG("Starting GPRS");
  String response = sendStringForOK("AT+CGATT?");
  while (response.indexOf('1') == -1) {
    LOG("GPRS not connected, waiting! 1");
    delay(1000);
    response = sendStringForOK("AT+CGATT?");
  }

  delay(1000);

  stopTCPConnection();

  response = sendStringForOK("AT+CIPMUX=0");
  while (response.indexOf('OK') == -1) {
    LOG("GPRS not connected, waiting! 2");
    delay(1000);
    response = sendStringForOK("AT+CIPMUX=0");
  }

  sendStringForOK("AT+CSTT=\"" + accessPoint + "\",\"\",\"\"");
  sendStringForOK("AT+CIICR");
  sendString("AT+CIFSR");
  String ip = readString();

  LOG("GPRS Connected as " + ip);
}

int SIM800::startTCPConnection(String server, int port) {
  sendStringForOK("AT+CIPSHUT");
  delay(1000);
  String response = sendStringForOK("AT+CIPSTART=\"TCP\",\"" + server + "\",\"" + port + "\"");
  int tries = 0;
  do {
    delay(1000);
    response = readString();
    tries++;
  } while (response.indexOf("CONNECT OK") == -1 && tries < 5);
  if (tries == 5) {
    return -1;
  }
  delay(1000);
  sendString("AT+CIPSEND");
  tries = 0;
  do {
    delay(1000);
    response = readString();
    tries++;
  } while (response.indexOf(">") == -1 && tries < 5);
  if (tries == 5) {
    return -1;
  }
  return 0;
}

void SIM800::stopTCPConnection() {
  sendStringForOK("AT+CIPSHUT");
}


String SIM800::sendStringForOK(String s) {
  sendString(s);
  String response = readString();
  if (response.indexOf("ERROR") >= 0) {
    LOG("GOT ERROR!!!");
    ;
  }
  while (response.indexOf("OK") == -1) {
    sendString(s);
    response = readString();
  }
  return response;
}

void SIM800::sendString(String s) {
  LOG("Sending: " + s);
  Serial1.println(s + "\r");
  Serial1.flush();
  delay(500);
}

String SIM800::readString() {
  return readString(20);
}

String SIM800::readString(int maxWaitTime) {
  String response = String();
  int waitCount = 0;
  while (!Serial1.available() > 0 && maxWaitTime > 0) {
    delay(100);
    maxWaitTime--;
  }
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (c != '\r' && c != '\n') {
      response += c;
    }
  }
  LOG("Got response: " + response);
  return response;
}

