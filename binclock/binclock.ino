#include <SoftwareSerial.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <Wire.h> 

#define MAX_CMD_LENGTH 80
 
#define ledPin 13
#define rxPin 10
#define txPin 11

unsigned long tempReadInterval = 5000UL;
 
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
  readTemp();
}

void readTime() {
  static time_t currentTime;
  currentTime = RTC.get();
}

void readTemp() {
  static unsigned long previousMillis = 0;
  if (millis() - previousMillis > tempReadInterval) {
    previousMillis = millis();
    Serial.println(RTC.temperature() / 4.0);
  }
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
  if(strcmp(command, "on") == 0){
    digitalWrite(ledPin,1);
    btSerial.println("LED on Pin 13 is on");
  }
  if (strcmp(command, "off") == 0){
    digitalWrite(ledPin,0);
    btSerial.println("LED on Pin 13 is off");
  }
}
