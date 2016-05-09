#include <SIM800.h>

SIM800Core  core;
SIM800GSM GSM800;

#define LOG(x) Serial.print(millis()); Serial.print(' '); Serial.println(x);

#define SINGLE_RUN_AND_PIPE

#define SERVER "dietervdw.dyndns.org"
#define PORT 12345

#define REPORT_DELAY 15000

#define PIN 6776

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Started serial!");

  pinMode(11, OUTPUT);

  digitalWrite(11, LOW);

  delay(1200);

  digitalWrite(11, HIGH);
  
  
  Serial1.begin(115200);
  while (!Serial1);
  Serial.println("Started serial1!");

  core.host.echo(false);

  waitForGSMModule();

  delay(1000);

  // checkPIN();

  // setupGPRS();
}

void waitForGSMModule() {
  boolean ok = false;

  while (!false) {
    LOG("Testing GSM module presence ...");
    sendString("AT");
    delay(500);
    if (Serial1.available() > 0) {
      String response = readString();
      if (response == "OK") {
        break;
      }
    } else {
      delay(1000);
    }
  }
  Serial.println("GSM Module OK!");
}

void checkPIN() {
  sendString("AT+CPIN?");
  String response = readString();

  if (response.indexOf("READY") == -1) {
    LOG("Entering PIN");
    sendString("AT+CPIN=" + PIN);
    response = readString();

    sendString("AT+CPIN?");
    if (response.indexOf("READY") == -1) {
      LOG("Could not verify PIN! Aborting!");
      abort();
    }
  }

  LOG("PIN OK");
}

void setupGPRS() {
  LOG("Starting GPRS");
  sendString("AT+CGATT?");
  String response = readString();
  while (response.indexOf('1') == -1) {
    LOG("GPRS not connected, waiting! 1");
    delay(1000);
    sendString("AT+CGATT?");
    String response = readString();
  }

  delay(1000);
  sendString("AT+CIPMUX=0");
  response = readString();
  while (response.indexOf('OK') == -1) {
    LOG("GPRS not connected, waiting! 1");
    delay(1000);
    sendString("AT+CIPMUX=0");
    response = readString();
  }

  sendString("AT+CSTT=\“internet.proximus.be\”,\“\”,“\”"); readString();
  sendString("AT+CIICR"); readString();
  sendString("AT+CIFSR");
  String ip = readString();

  LOG("GPRS Connected as " + ip);
}

void loop ()
{
//  LOG("Starting TCP connection...");
//  startTCPConnection(SERVER, PORT);
//
//  int sensorValue = analogRead(0);
//  Serial.print("Sending value: "); Serial.println(sensorValue);
//
//  LOG("Sending sensor value");
//  sendSensorValue(sensorValue);
//  LOG("Sensor value sent!");
//
//  stopTCPConnection();
//  LOG("TCP connection shut down!");
//  Serial.println();

#ifdef SINGLE_RUN_AND_PIPE
  while (1) {
    pipeInOut();
  }
#endif

  delay(REPORT_DELAY);
}

void sendString(String s) {
  Serial1.println(s + "\r");
}

String readString() {
  String response = String();
  while (!Serial1.available() > 0) {
    delay(100);
  }
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (c != '\r' && c != '\n') {
      response += c;
    }
  }
  Serial.print("Got response: ");
  Serial.println(response);
  return response;
}

void startTCPConnection(String server, int port) {
  sendString("AT+CIPSHUT"); readString();
  delay(1000);
  sendString("AT+CIPSTART=\”TCP\”,\”" + server + "\”,\”" + String(port) + "\”");
  String response = readString();
  while (response.indexOf('CONNECTED') == -1) {
    LOG("TCP connection not connected, waiting! 1");
    delay(1000);
    sendString("AT+CIPSTART=\”TCP\”,\”" + server + "\”,\”" + String(port) + "\”");
    response = readString();
  }
  sendString("AT+CIPSEND"); readString();
}

void stopTCPConnection() {
  sendString("\zAT+CIPSHUT"); readString();
}

void sendSensorValue(int value) {
  sendString("The value is: " + String(value) + "\r\n");
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

