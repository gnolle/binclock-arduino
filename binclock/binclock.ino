#include <SoftwareSerial.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <Timezone.h>
#include <Wire.h>
#include <FastLED.h>

#define NUM_LEDS 24
#define NUM_LED_ROWS 4
#define NUM_LED_COLS 6
#define DATA_PIN 7
#define TIME_INTERVAL 1000
#define FADE_INTERVAL 34
#define FADE_DURATION 800

const byte FADE_STEP = 255 * FADE_INTERVAL / FADE_DURATION;

const byte pows[] = {1, 2, 4, 8};

const byte ledPositions[4][6] = {
  {0, 1, 2, 3, 4, 5},
  {11, 10, 9, 8, 7, 6},
  {12, 13, 14, 15, 16, 17},
  {23, 22, 21, 20, 19, 18}
};

// first digit
const byte d1[4] = {0, 11, 12, 23};

// second digit
const byte d2[4] = {1, 10, 13, 22};

// minute first digit
const byte d3[4] = {2, 9, 14, 21};

// minute second digit
const byte d4[4] = {3, 8, 15, 20};

// second first digit
const byte d5[4] = {4, 7, 16, 19};

// second second digit
const byte d6[4] = {5, 6, 17, 18};


#define MAX_CMD_LENGTH 80

#define rxPin 11
#define txPin 10

byte singleClockColor[3];

byte rowColors[4][3] = {
  {0, 255, 255},
  {96, 255, 255},
  {140, 255, 255},
  {213, 255, 255}
};

byte displayMode = 0;

byte onOffMatrix[4][6] = {0};
byte fadeMatrix[4][6] = {0};

CRGB leds[NUM_LEDS];

SoftwareSerial btSerial(rxPin, txPin);

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  setSyncProvider(RTC.get);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  initClockColor();
}

time_t getLocalTime() {
  static TimeChangeRule germanSummerTime = {"DEUS", Last, Sun, Mar, 2, 120};
  static TimeChangeRule germanWinterTime = {"DEUW", Last, Sun, Oct, 3, 60};
  static Timezone germanTime(germanSummerTime, germanWinterTime);
  time_t localTime = germanTime.toLocal(now());
  return localTime;
}

void initClockColor() {
  randomSeed(analogRead(0));
  singleClockColor[0] = random(256);
  singleClockColor[1] = 255;
  singleClockColor[2] = 255;
}

void loop() {
  handleSerialByte(btSerial.read());
  showTime();
}

void readTime() {
  char timeCharBuffer[20];
  sprintf(timeCharBuffer, "%lu", now());

  char timeResponse[50];
  strcpy(timeResponse, "TIM");
  strcat(timeResponse, timeCharBuffer);

  writeToBtSerial(timeResponse);
}

void showTime() {
  static unsigned long previousTimeMillis = 0;
  static unsigned long previousFadeMillis = 0;

  if (millis() - previousFadeMillis > FADE_INTERVAL) {
    previousFadeMillis = millis();
    calculateFadeMatrix();
    displayDigits();
  }

  if (millis() - previousTimeMillis > TIME_INTERVAL) {
    previousTimeMillis = millis();
    time_t localTime = getLocalTime();

    byte digit1 = hour(localTime) / 10;
    byte digit2 = hour(localTime) % 10;
    byte digit3 = minute(localTime) / 10;
    byte digit4 = minute(localTime) % 10;
    byte digit5 = second(localTime) / 10;
    byte digit6 = second(localTime) % 10;

    byte displayValues[] = {
      digit1,
      digit2,
      digit3,
      digit4,
      digit5,
      digit6
    };

    calculateOnOffMatrix(displayValues);
  }
}

void calculateOnOffMatrix(byte rowValues[6]) {
  memset(onOffMatrix, 0, sizeof(onOffMatrix));

  for (int i = NUM_LED_ROWS - 1; i >= 0; i--) {
    int currentBinaryPower = pows[i];
    if (rowValues[0] > 0 && rowValues[0] >= currentBinaryPower) {
      rowValues[0] -= currentBinaryPower;
      onOffMatrix[i][0] = 1;
    }
    if (rowValues[1] > 0 && rowValues[1] >= currentBinaryPower) {
      rowValues[1] -= currentBinaryPower;
      onOffMatrix[i][1] = 1;
    }

    if (rowValues[2] > 0 && rowValues[2] >= currentBinaryPower) {
      rowValues[2] -= currentBinaryPower;
      onOffMatrix[i][2] = 1;
    }
    if (rowValues[3] > 0 && rowValues[3] >= currentBinaryPower) {
      rowValues[3] -= currentBinaryPower;
      onOffMatrix[i][3] = 1;
    }

    if (rowValues[4] > 0 && rowValues[4] >= currentBinaryPower) {
      rowValues[4] -= currentBinaryPower;
      onOffMatrix[i][4] = 1;
    }
    if (rowValues[5] > 0 && rowValues[5] >= currentBinaryPower) {
      rowValues[5] -= currentBinaryPower;
      onOffMatrix[i][5] = 1;
    }
  }
}

void calculateFadeMatrix() {
  for (byte i = 0; i < NUM_LED_ROWS; i++) {
    for (byte j = 0; j < NUM_LED_COLS; j++) {
      if (onOffMatrix[i][j]) {
        if (FADE_STEP > 255 - fadeMatrix[i][j]) {
          fadeMatrix[i][j] = 255;
        } else {
          fadeMatrix[i][j] += FADE_STEP;
        }
        continue;
      }
      if (!onOffMatrix[i][j]) {
        if (fadeMatrix[i][j] < FADE_STEP) {
          fadeMatrix[i][j] = 0;
        } else {
          fadeMatrix[i][j] -= FADE_STEP;
        }
      }
    }
  }
}

void printOnOffMatrix() {
  for (byte i = 0; i < NUM_LED_ROWS; i++) {
    for (byte j = 0; j < NUM_LED_COLS; j++) {
      Serial.print(onOffMatrix[i][j]);
      Serial.print(" ");
    }
    Serial.println("\n");
  }
}

void printFadeMatrix() {
  for (byte i = 0; i < NUM_LED_ROWS; i++) {
    for (byte j = 0; j < NUM_LED_COLS; j++) {
      Serial.print(fadeMatrix[i][j]);
      Serial.print(" ");
    }
    Serial.println("\n");
  }
}

void displayDigits() {
  for (byte i = 0; i < NUM_LED_ROWS; i++) {
    for (byte j = 0; j < NUM_LED_COLS; j++) {
      leds[ledPositions[i][j]].setHSV(singleClockColor[0], singleClockColor[1], fadeMatrix[i][j]);
    }
  }
  FastLED.show();
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
    singleClockColor[i] = atoi(part);
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

  displayMode = atoi(extractedMode);
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
    switch (serialByte) {
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

