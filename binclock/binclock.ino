#include <SoftwareSerial.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <Wire.h> 

#define MAX_CMD_LENGTH 80

#define ledPin 13
#define rxPin 10
#define txPin 11

int sensorPin = A0;
unsigned long luxReadInterval = 5000UL;
 
SoftwareSerial btSerial(rxPin, txPin);
 
void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  btSerial.println("bluetooth available");
  pinMode(ledPin, OUTPUT);
}
 
void loop() {
  handleSerialByte(btSerial.read());
  //readTime();
  readLux();
}

void readTime() {
  static time_t currentTime;
  currentTime = RTC.get();
}

void readLux() {
  static unsigned long previousMillis = 0;
  if (millis() - previousMillis > luxReadInterval) {
    previousMillis = millis();
    Serial.println(analogRead(sensorPin));
  }
}

void readTemperatureSensor() {
    float sensorReading = RTC.temperature() / 4.0;

    // convert reading to char[]
    char floatCharBuffer[5];
    dtostrf(sensorReading, 1, 2, floatCharBuffer);

    // build response (i.e. "TMP19.75")
    char tempResponse[10] = "TMP"; 
    strcat(tempResponse, floatCharBuffer);
    
    Serial.println(tempResponse);
    writeToBtSerial(tempResponse);
}

void writeToBtSerial(char const *message) {
  // add CR and print to serial
  char response[strlen(message) + 1];
  strcpy(response, message);
  strcat(response, "\r");
  btSerial.print(response);
}

void handleSerialByte(int serialByte) {
  static byte pos = 0;
  static char buffer[MAX_CMD_LENGTH];

  if (serialByte > 0) {
    switch(serialByte) {
      case '\n':
        break;
      case '\r':
        handleCommand(buffer);
        pos = 0;
        break;
      default:
        if (pos < MAX_CMD_LENGTH - 1) {
          buffer[pos] = serialByte;
          pos++;
          buffer[pos] = '\0';
        }
    }
  }
}

void handleCommand(char *command) {
  Serial.println(command);
  btSerial.println(command);

  if (strcmp(command, "TMP") == 0) {
    readTemperatureSensor();
    return;
  }
  
  if (strcmp(command, "on") == 0){
    digitalWrite(ledPin,1);
    writeToBtSerial("LED on Pin 13 is on");
    return;
  }
  
  if (strcmp(command, "off") == 0){
    digitalWrite(ledPin,0);
    writeToBtSerial("LED on Pin 13 is off");
    return;
  }
}
