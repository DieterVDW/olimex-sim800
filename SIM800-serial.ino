#include <SoftwareSerial.h>

#include "Sensor.h"
#include "US100.h"
#include "SIM800.h"
#include "SIM800TemperatureSensor.h"
#include "SIM800BatterySensor.h"

//#define ENABLE_LED
//#define SINGLE_RUN_AND_PIPE
#define DO_SENSORS
#define DO_SEND
#define SIM800_SHUTDOWN

#define SERVER "dietervdw.ddns.net"
#define PORT 12345

#define REPORT_DELAY 30000 // 1h

#define DEFAULT_PIN 1234
#define ACCESS_POINT "internet.bmbpartner.be"

#define DEFAULT_GSM_POWERSWITCH_PIN 11
SIM800 sim800(DEFAULT_GSM_POWERSWITCH_PIN, DEFAULT_PIN);

#define US100_RX 8
#define US100_TX 9
US100 us100(US100_RX, US100_TX);

const int NUM_SENSORS = 4;
Sensor* sensors[NUM_SENSORS];

void setup()
{
  sensors[0] = new SIM800TemperatureSensor(&sim800);
  sensors[1] = new SIM800BatterySensor(&sim800);
  sensors[2] = new US100TemperatureSensor(&us100);
  sensors[3] = new US100DistanceSensor(&us100);

#ifdef ENABLE_LED
  pinMode(17, OUTPUT);
  digitalWrite(17, LOW);
#endif

  if (Serial) {
    Serial.begin(115200);
    while (!Serial);
    LOG("Started serial!");
  }

  Serial1.begin(115200);
  while (!Serial1);
  LOG("Started serial1!");

#ifndef SIM800_SHUTDOWN
  startupSIM800();
#endif
}

void startupSIM800() {
  int gsmModuleActive = 0;

  while (gsmModuleActive == 0) {
    for (int i = 0; i < 3; i++) {
      delay(500);
      gsmModuleActive += sim800.testActive();
    }
    if (gsmModuleActive == 0) {
      sim800.turnOn();
    }
  }
  LOG("GSM Module OK!");

  sim800.echoOff();

  delay(1000);

  sim800.checkPIN();

  sim800.enableGPRS(ACCESS_POINT);
}

void loop ()
{
#ifdef SIM800_SHUTDOWN
    startupSIM800();
#endif
  
#ifdef SINGLE_RUN_AND_PIPE
  while (1) {
    pipeInOut();
  }
#endif

#ifdef DO_SENSORS
  LOG("Reading sensors...");
  String content = "{";
  for (int i = 0; i < NUM_SENSORS; i++) {
    Sensor* sensor = sensors[i];
    String sensorName = sensor->getName();
    LOG("Reading sensor: " + sensorName);
    String value = sensor->getValue();
    LOG("Value: " + value);

    content += "\"" + sensorName + "\": \"" + value + "\",";
  }
  content += "}";
#endif

#ifdef DO_SEND
  LOG("Starting TCP connection...");
  int ret = sim800.startTCPConnection(SERVER, PORT);
  delay(1000);
  if (ret >= 0) {
    sendSensorValue(content, SERVER, PORT);
    delay(1000);
  }
  sim800.stopTCPConnection();
  delay(1000);
  LOG("TCP connection shut down!");
#endif

  LOG(' ');

#ifdef SIM800_SHUTDOWN
  sim800.turnOff();
#endif

  delay(REPORT_DELAY);
}

void sendSensorValue(String content, String server, int port) {
  String headers = "POST /thegist/device/1/data HTTP/1.1\n"
                   "Host: " + server + ":" + String(port) + "\n"
                   "Connection: keep-alive\n"
                   "Content-Length: " + String(content.length()) + "\n"
                   "Cache-Control: no-cache\n"
                   "User-Agent: Olimex Sensorbed 1.0\n"
                   "Content-Type: application/json\n"
                   "Accept: */*\n\n";
  LOG("Content:");
  LOG(content);
  LOG("Headers:");
  LOG(headers);
  sim800.send(headers);
  sim800.send(content);
  sim800.write(26);
  sim800.flush();
  sim800.readString();
}

void pipeInOut() {
  while (Serial1.available() > 0) {
    Serial.write(Serial1.read());
  }

  while (Serial.available() > 0) {
    int input = Serial.read();
    if (input == 126) {
      input = 26;
    }
    Serial1.write(input);
  }
}

