#include <SoftwareSerial.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <Wire.h>
#include <FastLED.h>

#define NUM_LEDS 24
#define DATA_PIN 6
#define INTERVAL 40 

#define MAX_CMD_LENGTH 80

#define rxPin 10
#define txPin 11

CRGB leds[NUM_LEDS];
unsigned long timer;
byte thetas[24];
 
SoftwareSerial btSerial(rxPin, txPin);
 
void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  setSyncProvider(RTC.get);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}
 
void loop() {
  handleSerialByte(btSerial.read());
  lightLeds();
}

void readTime() {
  time_t currentTime = now();

  Serial.println(currentTime);
  
  char timeCharBuffer[20];
  sprintf(timeCharBuffer, "%lu", currentTime);

  char timeResponse[50];
  strcpy(timeResponse, "TIM");
  strcat(timeResponse, timeCharBuffer);

  writeToBtSerial(timeResponse);
}

void lightLeds() {
  static byte phase = 0;
  static unsigned long previousMillis = 0;
  if (millis() - previousMillis > INTERVAL) {
    previousMillis = millis();
    FastLED.clear();
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].setHSV(255, 255, sin8(phase + thetas[i]));  
      //leds[i].setHSV(255, 255, map(sin8(phase + thetas[i]), 0, 255, lowerBoundary, upperBoundary));  
    }
    phase++;
    FastLED.show();
  }
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
    
    //Serial.println(tempResponse);
    writeToBtSerial(tempResponse);
}

void setTimeFromCommand(char const *command) {
  const byte SETTIME_CMD_LENGTH = 7;
  byte timestampDigits = strlen(command) - SETTIME_CMD_LENGTH;
  char extractedTime[20];
  strncpy(extractedTime, command + SETTIME_CMD_LENGTH, timestampDigits);
  extractedTime[timestampDigits] = '\0';

  long newTime = atol(extractedTime);
  setTime(newTime);
  RTC.set(newTime);
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

  if (strpre("SETTIME", command)) {
    setTimeFromCommand(command);
    return;
  }
  
  if (strcmp(command, "TEMP") == 0) {
    readTemperatureSensor();
    return;
  }

  if (strcmp(command, "TIME") == 0) {
    readTime();
    return;
  }
}

boolean strpre(const char *pre, const char *str) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

