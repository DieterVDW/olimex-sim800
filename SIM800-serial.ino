#define LOG(x) Serial.print(millis()); Serial.print(' '); Serial.println(x);

// #define SINGLE_RUN_AND_PIPE 1

#define SERVER "dietervdw.ddns.net"
#define PORT 12345

#define REPORT_DELAY 30000

#define PIN 6776

#define GSM_POWERSWITCH_PIN 11

void setup()
{
  pinMode(GSM_POWERSWITCH_PIN, OUTPUT);
  digitalWrite(GSM_POWERSWITCH_PIN, LOW); // High by default

  Serial.begin(115200);
  while (!Serial);
  Serial.println("Started serial!");

  Serial1.begin(115200);
  while (!Serial1);
  Serial.println("Started serial1!");

  int gsmModuleActive = 0;
  for (int i = 0; i < 3; i++) {
    delay(500);
    gsmModuleActive += testGSMModuleActive();
  }
  if (gsmModuleActive == 0) {
    turnOnGSMModule();
  }

  waitForGSMModule();
  sendStringForOK("ATE0"); // Host echo off

  delay(1000);

  checkPIN();

  setupGPRS();
}

void waitForGSMModule() {
  while (!testGSMModuleActive()) {
    delay(1000);
  }
  LOG("GSM Module OK!");
}

void turnOnGSMModule() {
  LOG("Trying to turn on GSM module!");
  digitalWrite(GSM_POWERSWITCH_PIN, HIGH);
  delay(1200);
  digitalWrite(GSM_POWERSWITCH_PIN, LOW); // High by default
}

void turnOffGSMModule() {
  LOG("Shutting down GSM module!");
  sendStringForOK("AT+CPOWD=1");
}

int testGSMModuleActive() {
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

void checkPIN() {
  String response = sendStringForOK("AT+CPIN?");

  if (response.indexOf("READY") == -1) {
    LOG("Entering PIN");
    String command = "AT+CPIN=";
    response = sendStringForOK(command + PIN);

    response = sendStringForOK("AT+CPIN?");
    if (response.indexOf("READY") == -1) {
      LOG("Could not verify PIN! Aborting!");
      ;
    }
  }

  LOG("PIN OK");
}

void setupGPRS() {
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

  sendStringForOK("AT+CSTT=\"internet.proximus.be\",\"\",\"\"");
  sendStringForOK("AT+CIICR");
  sendString("AT+CIFSR");
  String ip = readString();

  LOG("GPRS Connected as " + ip);
}

void loop ()
{
  LOG("Starting TCP connection...");
  int ret = startTCPConnection(SERVER, PORT);
  if (ret >= 0) {
    int sensorValue = analogRead(0);
    String msg = "Sending value: ";
    LOG(msg + sensorValue);

    LOG("Sending sensor value");
    sendSensorValue(sensorValue);
    LOG("Sensor value sent!");
  }

  stopTCPConnection();
  LOG("TCP connection shut down!");
  Serial.println();

#ifdef SINGLE_RUN_AND_PIPE
  while (1) {
    pipeInOut();
  }
#endif

  delay(REPORT_DELAY);
}

String sendStringForOK(String s) {
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

void sendString(String s) {
  LOG("Sending: " + s);
  Serial1.println(s + "\r");
  Serial1.flush();
  delay(500);
}

String readString() {
  return readString(20);
}

String readString(int maxWaitTime) {
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

int startTCPConnection(String server, int port) {
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

void stopTCPConnection() {
  sendStringForOK("AT+CIPSHUT");
}

void sendSensorValue(int value) {
  sendString("The value is: " + String(value) + "\r\n");
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

