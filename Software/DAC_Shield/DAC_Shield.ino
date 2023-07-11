#include "DAC_Shield.h"

#include <CommandParser.h>  //Include the command parser library. https://github.com/Uberi/Arduino-CommandParser/tree/master

typedef CommandParser<> MyCommandParser;

MyCommandParser parser;




void setup() {
  Serial.begin(9600);
  while (!Serial);

  parser.registerCommand("TEST", "sdiu", &cmd_test);
  Serial.println("registered command: TEST <string> <double> <int64> <uint64>");
  Serial.println("example: TEST \"\\x41bc\\ndef\" -1.234e5 -123 123");
  parser.registerCommand("RESET", "", &cmdReset);
  Serial.println("registered command: RESET");
  Serial.println("example: RESET");
  parser.registerCommand("STATUS", "", &cmdStatus);
  Serial.println("registered command: STATUS");
  Serial.println("example: STATUS");
  parser.registerCommand("*IDN?", "", &cmdIdentify);
  Serial.println("registered command: *IDN?");
  Serial.println("example: *IDN?");
  parser.registerCommand("SET", "ids", &cmdSetChannelCurrent);
  Serial.println("registered command: SET <int> <double> <string>");
  Serial.println("example SET 1 1.24 uA");
  parser.registerCommand("SCALE", "s", &cmdScale);
  Serial.println("registered command: SCALE <string>");
  Serial.println("example SCALE +5");
}

void loop() {
  if (Serial.available()) {
    char line[128];
    size_t lineLength = Serial.readBytesUntil('\n', line, 127);
    line[lineLength] = '\0';

    char response[MyCommandParser::MAX_RESPONSE_SIZE];
    parser.processCommand(line, response);
    Serial.println(response);
  }
}