#include <SoftwareSerial.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <Wire.h>
#include <FastLED.h>

#define NUM_LEDS 24
#define NUM_LED_ROWS 4
#define DATA_PIN 7
#define TIME_INTERVAL 1000

const byte pows[] = {1, 2, 4, 8};

// hour first digit
const byte hour_d1[4] = {0, 11, 12, 23};

// hour second digit
const byte hour_d2[4] = {1, 10, 13, 22};

// minute first digit
const byte minute_d1[4] = {2, 9, 14, 21};

// minute second digit
const byte minute_d2[4] = {3, 8, 15, 20};

// second first digit
const byte second_d1[4] = {4, 7, 16, 19};

// second second digit
const byte second_d2[4] = {5, 6, 17, 18};


#define MAX_CMD_LENGTH 80

#define rxPin 11
#define txPin 10

byte clockColor[] = {255, 255, 255};
byte brightness = 64;

byte mode = 0;

CRGB leds[NUM_LEDS];
unsigned long timer;
 
SoftwareSerial btSerial(rxPin, txPin);
 
void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  setSyncProvider(RTC.get);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
}
 
void loop() {
  handleSerialByte(btSerial.read());
  if (mode == 0) {
    showTime();
  } else {
    showTimeAndTemperature();
  }
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
      int currentBinaryPower = pows[i];
      if (remainingHourDigit1 > 0 && remainingHourDigit1 >= currentBinaryPower) {
         remainingHourDigit1 -= currentBinaryPower;
         leds[hour_d1[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }
      if (remainingHourDigit2 > 0 && remainingHourDigit2 >= currentBinaryPower) {
         remainingHourDigit2 -= currentBinaryPower;
         leds[hour_d2[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }     
  
      if (remainingMinuteDigit1 > 0 && remainingMinuteDigit1 >= currentBinaryPower) {
         remainingMinuteDigit1 -= currentBinaryPower;
         leds[minute_d1[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }
      if (remainingMinuteDigit2 > 0 && remainingMinuteDigit2 >= currentBinaryPower) {
         remainingMinuteDigit2 -= currentBinaryPower;
         leds[minute_d2[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }    
       
      if (remainingSecondDigit1 > 0 && remainingSecondDigit1 >= currentBinaryPower) {
         remainingSecondDigit1 -= currentBinaryPower;
         leds[second_d1[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }
      if (remainingSecondDigit2 > 0 && remainingSecondDigit2 >= currentBinaryPower) {
         remainingSecondDigit2 -= currentBinaryPower;
         leds[second_d2[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }    
    }
    FastLED.show();
  }
}

void showTimeAndTemperature() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > TIME_INTERVAL) {
    previousMillis = millis();
    FastLED.clear();

    float sensorReading = RTC.temperature() / 4.0;
    byte roundedTemperature = (byte) (sensorReading + 0.5);

    byte remainingHourDigit1 = (hour() + 1) / 10;
    byte remainingHourDigit2 = (hour() + 1) % 10;
    byte remainingMinuteDigit1 = minute() / 10;
    byte remainingMinuteDigit2 = minute() % 10;
    byte remainingTemperatureDigit1 = roundedTemperature / 10;
    byte remainingTemperatureDigit2 = roundedTemperature % 10;
  
    for (int i = NUM_LED_ROWS - 1; i >= 0; i--) {
      int currentBinaryPower = pows[i];
      if (remainingHourDigit1 > 0 && remainingHourDigit1 >= currentBinaryPower) {
         remainingHourDigit1 -= currentBinaryPower;
         leds[hour_d1[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }
      if (remainingHourDigit2 > 0 && remainingHourDigit2 >= currentBinaryPower) {
         remainingHourDigit2 -= currentBinaryPower;
         leds[hour_d2[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }     
  
      if (remainingMinuteDigit1 > 0 && remainingMinuteDigit1 >= currentBinaryPower) {
         remainingMinuteDigit1 -= currentBinaryPower;
         leds[minute_d1[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }
      if (remainingMinuteDigit2 > 0 && remainingMinuteDigit2 >= currentBinaryPower) {
         remainingMinuteDigit2 -= currentBinaryPower;
         leds[minute_d2[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }    
       
      if (remainingTemperatureDigit1 > 0 && remainingTemperatureDigit1 >= currentBinaryPower) {
         remainingTemperatureDigit1 -= currentBinaryPower;
         leds[second_d1[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }
      if (remainingTemperatureDigit2 > 0 && remainingTemperatureDigit2 >= currentBinaryPower) {
         remainingTemperatureDigit2 -= currentBinaryPower;
         leds[second_d2[i]].setHSV(clockColor[0], clockColor[1], clockColor[2]);
       }    
    }
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

void setColorFromCommand(char const *command) {
  const byte SETCOLOR_CMD_LENGTH = 8;
  byte hsvDigits = strlen(command) - SETCOLOR_CMD_LENGTH;
  char extractedHsv[20];
  strncpy(extractedHsv, command + SETCOLOR_CMD_LENGTH, hsvDigits);
  extractedHsv[hsvDigits] = '\0';

  char delimiter[] = ",";
  char *part;
  part = strtok(extractedHsv, delimiter);

  byte i = 0;
  while (part != NULL) {
    clockColor[i] = atoi(part);
    part = strtok(NULL, delimiter);
    i++;
  }

}

void setModeFromCommand(char const *command) {
  const byte SETMODE_CMD_LENGTH = 7;
  byte modeDigits = strlen(command) - SETMODE_CMD_LENGTH;
  char extractedMode[5];
  strncpy(extractedMode, command + SETMODE_CMD_LENGTH, modeDigits);
  extractedMode[modeDigits] = '\0';

  mode = atoi(extractedMode);
}

void setBrightnessFromCommand(char const *command) {
  const byte SETBRIGHT_CMD_LENGTH = 9;
  byte brightDigits = strlen(command) - SETBRIGHT_CMD_LENGTH;
  char extractedBrightness[4];
  strncpy(extractedBrightness, command + SETBRIGHT_CMD_LENGTH, brightDigits);
  extractedBrightness[brightDigits] = '\0';

  brightness = atoi(extractedBrightness);
  FastLED.setBrightness(brightness);
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

  if (strpre("SETMODE", command)) {
    setModeFromCommand(command);
    return;
  }

  if (strpre("SETBRIGHT", command)) {
    setBrightnessFromCommand(command);
    return;
  }  

  if (strpre("SETTIME", command)) {
    setTimeFromCommand(command);
    return;
  }

  if (strpre("SETCOLOR", command)) {
    setColorFromCommand(command);
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

