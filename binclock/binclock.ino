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
  setSyncProvider(RTC.get);
  pinMode(ledPin, OUTPUT);
}
 
void loop() {
  handleSerialByte(btSerial.read());
  //readTime();
  readLight();
}

void readTime() {
  time_t currentTime = now();
  char timeCharBuffer[20];
  itoa(currentTime, timeCharBuffer, 10);

  char timeResponse[50];
  strcpy(timeResponse, "TIM");
  strcat(timeResponse, timeCharBuffer);
    
  writeToBtSerial(timeResponse);
}

void readLight() {
  static unsigned long previousMillis = 0;
  if (millis() - previousMillis > luxReadInterval) {
    previousMillis = millis();
    Serial.println(analogRead(sensorPin));
  }
}

void readBrightnessSensor() {
  int brightness = analogRead(sensorPin);
  char brightnessCharBuffer[20];
  itoa(brightness, brightnessCharBuffer, 10);
  writeToBtSerial(brightnessCharBuffer);
}

void readTemperatureSensor() {
    float sensorReading = RTC.temperature() / 4.0;

    // convert reading to char[]
    char floatCharBuffer[20];
    dtostrf(sensorReading, 1, 2, floatCharBuffer);

    Serial.println(floatCharBuffer);

    // build response (i.e. "TMP19.75")
    char tempResponse[50];
    strcpy(tempResponse, "TMP");
    strcat(tempResponse, floatCharBuffer);

    Serial.println(tempResponse);
    
    //Serial.println(tempResponse);
    writeToBtSerial(tempResponse);
}

void setTimeFromCommand(char const *command) {
  const byte SETTIME_CMD_LENGTH = 7;
  byte timestampDigits = strlen(command) - SETTIME_CMD_LENGTH);
  char extractedTime[20];
  strncpy(extractedTime, command + SETTIME_CMD_LENGTH, timestampDigits);
  extractedTime[timestampDigits] = '\0';
  Serial.print(extractedTime);
}

void writeToBtSerial(char const *message) {
  // add CR and print to serial
  char response[50];
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

  if (strpre(command, "SETTIME")) {
    setTimeFromCommand(command);
    return
  }
  
  if (strcmp(command, "TEMP") == 0) {
    readTemperatureSensor();
    return;
  }

  if (strcmp(command, "TIME") == 0) {
    readTime();
    return;
  }

  if (strcmp(command, "LIGHT") == 0) {
    readBrightnessSensor();
    return;
  }
  
  if (strcmp(command, "on") == 0){
    digitalWrite(ledPin,1);
    writeToBtSerial("LED on üa Pinß 1ü3 is on");
    return;
  }
  
  if (strcmp(command, "off") == 0){
    digitalWrite(ledPin,0);
    writeToBtSerial("LED on Pin 13 is off");
    return;
  }
}

boolean strpre(const char *pre, const char *str) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

