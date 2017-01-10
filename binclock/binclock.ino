#include <SoftwareSerial.h>

#define MAX_CMD_LENGTH 20
 
#define ledPin 13
#define rxPin 10
#define txPin 11
 
SoftwareSerial btSerial(rxPin, txPin);
 
void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  btSerial.println("bluetooth available");
  pinMode(ledPin, OUTPUT);
}
 
void loop() {
  handleSerialByte(btSerial.read());
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
  if(strcmp(command, "on") == 0){
    digitalWrite(ledPin,1);
    btSerial.println("LED on Pin 13 is on");
  }
  if (strcmp(command, "off") == 0){
    digitalWrite(ledPin,0);
    btSerial.println("LED on Pin 13 is off");
  }
}
