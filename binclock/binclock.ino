#include <SoftwareSerial.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <Wire.h>
#include <FastLED.h>

#define NUM_LEDS 24
#define NUM_LED_ROWS 4
#define DATA_PIN 6
#define INTERVAL 40 
#define TIME_INTERVAL 1000

// hour first digit
byte hour_d1[4];

// hour second digit
byte hour_d2[4];

// minute first digit
byte minute_d1[4];

// minute second digit
byte minute_d2[4];

// second first digit
byte second_d1[4];

// second second digit
byte second_d2[4];


#define MAX_CMD_LENGTH 80

#define rxPin 10
#define txPin 11

CRGB leds[NUM_LEDS];
unsigned long timer;
byte thetas[24];
 
SoftwareSerial btSerial(rxPin, txPin);
 
void setup() {
  initLedIndex();
  Serial.begin(9600);
  btSerial.begin(9600);
  setSyncProvider(RTC.get);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void initLedIndex() {
  hour_d1[0] = 0;
  hour_d1[1] = 11;
  hour_d1[2] = 12;
  hour_d1[3] = 23;  
  hour_d2[0] = 1;
  hour_d2[1] = 10;
  hour_d2[2] = 13;
  hour_d2[3] = 22;
  minute_d1[0] = 2;
  minute_d1[1] = 9;
  minute_d1[2] = 14;
  minute_d1[3] = 21;  
  minute_d2[0] = 3;
  minute_d2[1] = 8;
  minute_d2[2] = 15;
  minute_d2[3] = 20;
  second_d1[0] = 4;
  second_d1[1] = 7;
  second_d1[2] = 16;
  second_d1[3] = 19;
  second_d2[0] = 5;
  second_d2[1] = 6;
  second_d2[2] = 17;
  second_d2[3] = 18;
}
 
void loop() {
  handleSerialByte(btSerial.read());
  //lightLeds();
  showTime();
}

void readTime() {
  time_t currentTime = now();
  
  char timeCharBuffer[20];
  sprintf(timeCharBuffer, "%lu", currentTime);

  char timeResponse[50];
  strcpy(timeResponse, "TIM");
  strcat(timeResponse, timeCharBuffer);

  writeToBtSerial(timeResponse);
}

void showTime() {

  static unsigned long previousMillis = 0;
  if (millis() - previousMillis > TIME_INTERVAL) {
    previousMillis = millis();
    FastLED.clear();

    byte remainingHourDigit1 = (hour() + 1) / 10;
    byte remainingHourDigit2 = (hour() + 1) % 10;
    byte remainingMinuteDigit1 = minute() / 10;
    byte remainingMinuteDigit2 = minute() % 10;
    byte remainingSecondDigit1 = second() / 10;
    byte remainingSecondDigit2 = second() % 10;
  
    for (int i = NUM_LED_ROWS - 1; i >= 0; i--) {
      byte currentBinaryPower = pow(2, i);
      if (remainingHourDigit1 > 0 && remainingHourDigit1 >= currentBinaryPower) {
         remainingHourDigit1 -= currentBinaryPower;
         leds[hour_d1[i]].setHSV(255, 255, 255);
       }
      if (remainingHourDigit2 > 0 && remainingHourDigit2 >= currentBinaryPower) {
         remainingHourDigit2 -= currentBinaryPower;
         leds[hour_d2[i]].setHSV(255, 255, 255);
       }     
       
      if (remainingMinuteDigit1 > 0 && remainingMinuteDigit1 >= currentBinaryPower) {
         remainingMinuteDigit1 -= currentBinaryPower;
         leds[minute_d1[i]].setHSV(255, 255, 255);
       }
      if (remainingMinuteDigit2 > 0 && remainingMinuteDigit2 >= currentBinaryPower) {
         remainingMinuteDigit2 -= currentBinaryPower;
         leds[minute_d2[i]].setHSV(255, 255, 255);
       }    
       
      if (remainingSecondDigit1 > 0 && remainingSecondDigit1 >= currentBinaryPower) {
         remainingSecondDigit1 -= currentBinaryPower;
         leds[second_d1[i]].setHSV(255, 255, 255);
       }
      if (remainingSecondDigit2 > 0 && remainingSecondDigit2 >= currentBinaryPower) {
         remainingSecondDigit2 -= currentBinaryPower;
         leds[second_d2[i]].setHSV(255, 255, 255);
       }    
    }
    FastLED.show();
  }
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

