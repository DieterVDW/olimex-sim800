#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "Sensor.h"
#include "US100.h"
#include "SIM800.h"
#include "SIM800TemperatureSensor.h"
#include "SIM800BatterySensor.h"
#include "SIM800RTCSensor.h"

// #define ENABLE_LED
//#define SINGLE_RUN_AND_PIPE
#define DO_SENSORS
#define DO_SEND
#define SIM800_SHUTDOWN

#define SERVER "52.58.109.152"
#define PORT 8080

#define REPORT_DELAY 3600000 // 1h
#define SLEEP_WITH_WATCHDOG
#define WATCHDOG_CYCLES 444  // 1 cycle ~= 8s, 69 cycles ~= 10min, 444 cycles ~= 1h

#define DEFAULT_PIN 1234
#define ACCESS_POINT "internet.bmbpartner.be"

#define DEFAULT_GSM_POWERSWITCH_PIN 11
SIM800 sim800(DEFAULT_GSM_POWERSWITCH_PIN, DEFAULT_PIN);

#define US100_RX 8
#define US100_TX 9
US100 us100(US100_RX, US100_TX);

const int NUM_SENSORS = 5;
Sensor* sensors[NUM_SENSORS];

void setup()
{
  // disable ADC
  ADCSRA = 0;

  sensors[0] = new SIM800TemperatureSensor(&sim800);
  sensors[1] = new SIM800BatterySensor(&sim800);
  sensors[2] = new SIM800RTCSensor(&sim800);
  sensors[3] = new US100TemperatureSensor(&us100);
  sensors[4] = new US100DistanceSensor(&us100);

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
  content.setCharAt(content.length() - 1, '}');
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

  sleepUntilNextReading();
}

void sleepUntilNextReading() {
#ifdef SLEEP_WITH_WATCHDOG
  for (int cycles = 0; cycles < WATCHDOG_CYCLES; cycles++) {
    // clear various "reset" flags
    MCUSR = 0;
    // allow changes, disable reset
    WDTCSR = bit (WDCE) | bit (WDE);
    // set interrupt mode and an interval
    WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
    wdt_reset();  // pat the dog

    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
    noInterrupts ();           // timed sequence follows
    sleep_enable();

    interrupts ();             // guarantees next instruction executed
    sleep_cpu ();

    // cancel sleep as a precaution
    sleep_disable();
  }
#else
  delay(REPORT_DELAY);
#endif
}

#ifdef SLEEP_WITH_WATCHDOG
// watchdog interrupt
ISR (WDT_vect)
{
  wdt_disable();  // disable watchdog
}  // end of WDT_vect
#endif

void sendSensorValue(String content, String server, int port) {
  LOG(server);
  LOG(port);
  LOG(content.length());
  sim800.send("POST /thegist/device/2/sensor/data HTTP/1.1\n");
  sim800.send("Host: " + server + ":" + String(port) + "\n");
  sim800.send("Connection: keep-alive\n");
  sim800.send("Content-Length: " + String(content.length()) + "\n");
  sim800.send("Cache-Control: no-cache\n");
  sim800.send("User-Agent: Olimex Sensorbed 1.0\n");
  sim800.send("Content-Type: application/json\n");
  sim800.send("Accept: */*\n\n");
  LOG("Content:");
  LOG(content);
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

