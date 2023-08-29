#include <CommandParser.h>  //Include the command parser library. https://github.com/Uberi/Arduino-CommandParser/tree/master
#include <SPI.h> //Include the library for SPI

#include "DAC_Shield.h"



typedef CommandParser<> MyCommandParser;

MyCommandParser parser;

#define AD5754_MAX_SPI 30000000 //30MHz
#define CS_PIN 10
#define CLR_PIN 9
#define STATUS_PIN 8


void setup() {
  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);
  //Serial Setup
  Serial.begin(9600);
  while (!Serial);
  //SPI Setup
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLR_PIN, OUTPUT);
  
  digitalWrite(CLR_PIN, HIGH);
  digitalWrite(CS_PIN, HIGH);//De-assert CS pin
  SPI.begin();
  SPI.beginTransaction(SPISettings(AD5754_MAX_SPI, MSBFIRST, SPI_MODE0));
  SPI.setDataMode(SPI_MODE1);
  DAC_setup(true);  //Setup and enable outputs
  //SPI_Write(LOAD, 0x0000);
  //SPI_Write(CLEAR, 0x0007);
  delay(1000);
  parser.registerCommand("RESET", "", &cmdReset);
  //Serial.println(F("registered command: RESET"));
  //Serial.println(F("example: RESET"));
  parser.registerCommand("STATUS", "", &cmdStatus);
  //Serial.println(F("registered command: STATUS"));
  //Serial.println(F("example: STATUS"));
  parser.registerCommand("*IDN?", "", &cmdIdentify);
  //Serial.println(F("registered command: *IDN?"));
  //Serial.println(F("example: *IDN?"));
  parser.registerCommand("SET", "ids", &cmdSetChannelCurrent);
  //Serial.println(F("registered command: SET <int> <double> <string>"));
  //Serial.println(F("example SET 1 1.24 uA"));
  parser.registerCommand("GET", "i", &cmdGetChannelCurrent);
  //Serial.println(F("registered command: GET <int>"));
  //Serial.println(F("example GET 1"));
  parser.registerCommand("RANGE", "is", &cmdScale);
  //Serial.println(F("registered command: RANGE <int> <string>"));
  //Serial.println(F("example RANGE 1 +5"));
  parser.registerCommand("VMON", "", &cmdVmon);
  //Serial.println(F("registered command: VMON"));
  //Serial.println(F("example: VMON"));
  digitalWrite(STATUS_PIN, HIGH);
}

void loop() {
  if (Serial.available()) {
    char line[128];
    size_t lineLength = Serial.readBytesUntil('\n', line, 127);
    line[lineLength] = '\0';

    char response[MyCommandParser::MAX_RESPONSE_SIZE];
    parser.processCommand(strupr(line), response);
    Serial.println(response);
  }
}