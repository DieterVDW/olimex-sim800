#include <SoftwareSerial.h>

#include "Sensor.h"
#include "US100.h"
#include "SIM800.h"
#include "SIM800TemperatureSensor.h"

// #define SINGLE_RUN_AND_PIPE 1

#define ENABLE_LED

#define DO_SEND
#define SERVER "dietervdw.ddns.net"
#define PORT 12345

#define REPORT_DELAY 30000

SIM800 sim800;

#define US100_RX 8
#define US100_TX 9
US100 us100(US100_RX, US100_TX);

const int NUM_SENSORS = 3;
Sensor* sensors[NUM_SENSORS];

void setup()
{
  sensors[0] = new SIM800TemperatureSensor(&sim800);
  sensors[1] = new US100TemperatureSensor(&us100);
  sensors[2] = new US100DistanceSensor(&us100);

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

  int gsmModuleActive = 0;
  for (int i = 0; i < 3; i++) {
    delay(500);
    gsmModuleActive += sim800.testActive();
  }
  if (gsmModuleActive == 0) {
    sim800.turnOn();
  }

  waitForGSMModule();
  sim800.echoOff();

  delay(1000);

  //  checkPIN();
  //
  //  setupGPRS();
}

void waitForGSMModule() {
  while (!sim800.testActive()) {
    delay(1000);
  }
  LOG("GSM Module OK!");
}

void loop ()
{
#ifdef SINGLE_RUN_AND_PIPE
  while (1) {
    pipeInOut();
  }
#endif

#ifdef DO_SEND
  LOG("Starting TCP connection...");
  int ret = sim800.startTCPConnection(SERVER, PORT);
  if (ret >= 0) {
    String content = "{";
    for (int i = 0; i < NUM_SENSORS; i++) {
      Sensor* sensor = sensors[i];
      String sensorName = sensor->getName();
      LOG("Reading sensor: " + sensorName);
      String value = sensor->getValue();
      
      content += "\"" + sensorName + "\": \"" + value + "\",";
    }
    content += "}";
  }

  sim800.stopTCPConnection();
  LOG("TCP connection shut down!");
#endif

  LOG(' ');

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
  Serial1.print(headers + content);
  Serial1.write(26);
  Serial1.flush();
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

