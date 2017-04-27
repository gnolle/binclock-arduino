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

byte singleClockColor[3];

byte rowColors[4][3] = {
    {0, 255, 255},
    {96, 255, 255},
    {140, 255, 255},
    {213, 255, 255}
  };

byte displayMode = 0;
byte colorMode = 1;

CRGB leds[NUM_LEDS];
unsigned long timer;

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
  static TimeChangeRule germanWinterTime = {"WINS", Last, Sun, Oct, 3, 60};
  static Timezone germanTime(germanSummerTime, germanWinterTime);
  time_t localTime = germanTime.toLocal(now());
  return localTime;
}

void initClockColor() {
  singleClockColor[0] = random(256);
  singleClockColor[1] = 255;
  singleClockColor[2] = 255;
}

void loop() {
  handleSerialByte(btSerial.read());
  if (displayMode == 0) {
    showTime();
  } else {
    showTimeAndTemperature();
  }
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
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > TIME_INTERVAL) {
    previousMillis = millis();
    time_t localTime = getLocalTime();

    byte remainingHourDigit1 = hour(localTime) / 10;
    byte remainingHourDigit2 = hour(localTime) % 10;
    byte remainingMinuteDigit1 = minute(localTime) / 10;
    byte remainingMinuteDigit2 = minute(localTime) % 10;
    byte remainingSecondDigit1 = second(localTime) / 10;
    byte remainingSecondDigit2 = second(localTime) % 10;

    byte displayValues[] = {
      remainingHourDigit1,
      remainingHourDigit2,
      remainingMinuteDigit1,
      remainingMinuteDigit2,
      remainingSecondDigit1,
      remainingSecondDigit2
    };

    displayDigitsForRows(displayValues);
  }
}

void showTimeAndTemperature() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > TIME_INTERVAL) {
    previousMillis = millis();

    float sensorReading = RTC.temperature() / 4.0;
    byte roundedTemperature = (byte) (sensorReading + 0.5);

    time_t localTime = getLocalTime();

    byte remainingHourDigit1 = hour(localTime) / 10;
    byte remainingHourDigit2 = hour(localTime) % 10;
    byte remainingMinuteDigit1 = minute(localTime) / 10;
    byte remainingMinuteDigit2 = minute(localTime) % 10;
    byte remainingTemperatureDigit1 = roundedTemperature / 10;
    byte remainingTemperatureDigit2 = roundedTemperature % 10;

    byte displayValues[] = {
      remainingHourDigit1,
      remainingHourDigit2,
      remainingMinuteDigit1,
      remainingMinuteDigit2,
      remainingTemperatureDigit1,
      remainingTemperatureDigit2
    };

    displayDigitsForRows(displayValues);
  }
}

void displayDigitsForRows(byte rowValues[6]) {
  colorBackgroundLeds();

  for (int i = NUM_LED_ROWS - 1; i >= 0; i--) {
    int currentBinaryPower = pows[i];
    if (rowValues[0] > 0 && rowValues[0] >= currentBinaryPower) {
      rowValues[0] -= currentBinaryPower;
      colorLedAtPosition(hour_d1[i]);
    }
    if (rowValues[1] > 0 && rowValues[1] >= currentBinaryPower) {
      rowValues[1] -= currentBinaryPower;
      colorLedAtPosition(hour_d2[i]);
    }

    if (rowValues[2] > 0 && rowValues[2] >= currentBinaryPower) {
      rowValues[2] -= currentBinaryPower;
      colorLedAtPosition(minute_d1[i]);
    }
    if (rowValues[3] > 0 && rowValues[3] >= currentBinaryPower) {
      rowValues[3] -= currentBinaryPower;
      colorLedAtPosition(minute_d2[i]);
    }

    if (rowValues[4] > 0 && rowValues[4] >= currentBinaryPower) {
      rowValues[4] -= currentBinaryPower;
      colorLedAtPosition(second_d1[i]);
    }
    if (rowValues[5] > 0 && rowValues[5] >= currentBinaryPower) {
      rowValues[5] -= currentBinaryPower;
      colorLedAtPosition(second_d2[i]);
    }
  }
  FastLED.show();
}

void colorBackgroundLeds() {
    switch (colorMode) {
    case 0:
      fill_solid(leds, NUM_LEDS, CHSV(singleClockColor[0], singleClockColor[1], singleClockColor[2] / 2));
      break;
    case 1:
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].setHSV(rowColors[i / NUM_LED_COLS][0], rowColors[i / NUM_LED_COLS][1], rowColors[i / NUM_LED_COLS][2] / 2);
      }
      break;
  }
}

void colorLedAtPosition(int position) {
  switch (colorMode) {
    case 0:
      singleColorLed(position);
      break;
    case 1:
      rowColorLed(position);
      break;      
  }
}

void singleColorLed(int position) {
  leds[position].setHSV(singleClockColor[0], singleClockColor[1], singleClockColor[2]);
}

void rowColorLed(int position) {
  leds[position].setHSV(rowColors[position / NUM_LED_COLS][0], rowColors[position / NUM_LED_COLS][1], rowColors[position / NUM_LED_COLS][2]);
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

void setCodeFromCommand(char const *command) {
  const byte SETCODE_CMD_LENGTH = 7;
  byte codeDigits = strlen(command) - SETCODE_CMD_LENGTH;
  char extractedCode[5];
  strncpy(extractedCode, command + SETCODE_CMD_LENGTH, codeDigits);
  extractedCode[codeDigits] = '\0';

  colorMode = atoi(extractedCode);
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

  if (strpre("SETCODE", command)) {
    setCodeFromCommand(command);
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

