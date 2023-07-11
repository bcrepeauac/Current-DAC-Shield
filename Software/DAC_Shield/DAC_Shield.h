#ifndef _DACSHIELD_H    // Put these two lines at the top of your file.
#define _DACSHIELD_H    // (Use a suitable name, usually based on the file name.)

#include <CommandParser.h>  //Include the command parser library. https://github.com/Uberi/Arduino-CommandParser/tree/master

#define IDN_string "Amherst College, Current DAC, SN001, A00.01.00"
#define VREF 2.5
#define RESOLUTION 16

#define CH1_RESISTOR 50000  //RN3 on Circuit Board
#define CH2_RESISTOR 27000  //RN4 on Circuit Board
#define CH3_RESISTOR 20000  //RN1 on Circuit Board
#define CH4_RESISTOR 1000   //RN2 on Circuit Board

typedef CommandParser<> MyCommandParser;

int16_t Channel1Setting = 0;
int16_t Channel2Setting = 0;
int16_t Channel3Setting = 0;
int16_t Channel4Setting = 0;
uint8_t Channel1Gain = 4;
uint8_t Channel2Gain = 4;
uint8_t Channel3Gain = 4;
uint8_t Channel4Gain = 4;
bool Channel1Bipolar = 1;
bool Channel2Bipolar = 1;
bool Channel3Bipolar = 1;
bool Channel4Bipolar = 1;
double Channel1Setpoint = 0.0;
double Channel2Setpoint = 0.0;
double Channel3Setpoint = 0.0;
double Channel4Setpoint = 0.0;
// Helper functions

uint16_t float2bin(double inputValue, uint16_t resistance, bool bipolar, uint8_t gain){
  // convert float to the 2's compliment 
  double temp = resistance*inputValue;
  temp /= VREF*gain;
  if (bipolar){
    temp += 0.5;
  }
  
  int32_t binary = round((pow(2, RESOLUTION)-1)*temp);
  return (uint16_t)constrain(binary, 0, pow(2, RESOLUTION)-1);  //Return the constained values
}

double bin2float(uint16_t binary, uint16_t resistance, bool bipolar, uint8_t gain){
  //Convert 2's compliment to analog value
  double temp = binary/(pow(2,RESOLUTION)-1);
  temp *= VREF*gain;
  
  if (bipolar){
    temp -= gain*VREF/2;
  }
  return (temp/resistance);
}

// Command Functions
void cmdReset(MyCommandParser::Argument *args, char *response){
  // Reset all outputs to Zero
  strlcpy(response, "OK", MyCommandParser::MAX_RESPONSE_SIZE);
}
void cmdSetChannelCurrent(MyCommandParser::Argument *args, char *response){

  uint32_t resistor;
  uint8_t gain;
  bool bipolar;
  double * setpoint_ptr = NULL;

  //Parse the input args:
  uint8_t channel = (uint8_t)args[0].asInt64;
  Serial.print("Channel: ");
  Serial.println(channel);
  double setpoint = args[1].asDouble;
  Serial.print("Setting: ");
  Serial.println(setpoint);
  char * units = args[2].asString;
  Serial.print("Units: ");
  
  //Switch case to determine units
  if (strcmp(units, "uA") == 0){
    
    setpoint *= 1e-6;
    //Serial.println(setpoint, 8);
  }
  else if (strcmp(units, "mA") == 0){
    
    setpoint *= 1e-3;
    //Serial.println(setpoint, 5);
  }
  else if (strcmp(units, "A") == 0){
    
    setpoint *= 1;
    //Serial.println(setpoint, 2);
  }
  else{ // if no match is found, report an error
    strlcpy_PF(response, F("ERROR: Unit not found, options are A, mA, uA"), MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  //Set channel parameters
  switch (channel){
    case 1:
      resistor = CH1_RESISTOR;
      gain = Channel1Gain;
      bipolar = Channel1Bipolar;
      break;
    case 2:
      resistor = CH2_RESISTOR;
      gain = Channel2Gain;
      bipolar = Channel2Bipolar;
      break;
    case 3:
      resistor = CH3_RESISTOR;
      gain = Channel3Gain;
      bipolar = Channel3Bipolar;
      break;
    case 4:
      resistor = CH4_RESISTOR;
      gain = Channel4Gain;
      bipolar = Channel4Bipolar;
      break;
    default:
      strlcpy_PF(response, F("ERROR: Channel not found, options are 1, 2, 3, and 4"), MyCommandParser::MAX_RESPONSE_SIZE);
      return;
  }
  //Calculate DAC Command from float
  uint16_t binary = float2bin(setpoint, resistor, bipolar, gain);
  Serial.println(binary, BIN);
  //Send Commands to DAC
  //Report actual current
  double current = bin2float(binary, resistor, bipolar, gain);
  //*setpoint_ptr = current;
  if (current > 1){
    Serial.print(current, 4);
    Serial.println("A");
  }
  else if (current > 1e-3){
    Serial.print(current*1e3, 4);
    Serial.println("mA");
  }
  else{
    Serial.print(current*1e6, 4);
    Serial.println("uA");
  }
  strlcpy(response, "OK", MyCommandParser::MAX_RESPONSE_SIZE);
}
void cmdGetChannelCurrent(MyCommandParser::Argument *args, char *response){
  uint8_t channel = (uint8_t)args[0].asInt64;
  double setpoint = 0;
  switch (channel){
    case 1:
      setpoint = Channel1Setpoint;
      break;
    case 2:
      setpoint = Channel2Setpoint;
      break;
    case 3:
      setpoint = Channel3Setpoint;
      break;
    case 4:
      setpoint = Channel4Setpoint;
      break;
    default:
      strlcpy_PF(response, F("ERROR: Channel not found, options are 1, 2, 3, and 4"), MyCommandParser::MAX_RESPONSE_SIZE);
      return;
  }
  if (setpoint > 1){
    Serial.print(setpoint, 4);
    Serial.println("A");
  }
  else if (setpoint > 1e-3){
    Serial.print(setpoint*1e3, 4);
    Serial.println("mA");
  }
  else{
    Serial.print(setpoint*1e6, 4);
    Serial.println("uA");
  }
  strlcpy(response, "OK", MyCommandParser::MAX_RESPONSE_SIZE);
}
void cmdStatus(MyCommandParser::Argument *args, char *response){
  // Print out current settings
  Serial.println(Channel1Setting);

}
void cmdIdentify(MyCommandParser::Argument *args, char *response){
  // Print out IDN String
  strlcpy_PF(response, F(IDN_string), MyCommandParser::MAX_RESPONSE_SIZE);  // copy from flash memory to save space
}
void cmdScale(MyCommandParser::Argument *args, char *response){
  //Figure out what scale to change to

  //Set the channel scale

  //Update the channel values

}





void cmd_test(MyCommandParser::Argument *args, char *response) {
  Serial.print("string: "); Serial.println(args[0].asString);
  Serial.print("double: "); Serial.println(args[1].asDouble);
  Serial.print("int64: "); Serial.println((int32_t)args[2].asInt64); // NOTE: on older AVR-based boards, Serial doesn't support printing 64-bit values, so we'll cast it down to 32-bit
  Serial.print("uint64: "); Serial.println((uint32_t)args[3].asUInt64); // NOTE: on older AVR-based boards, Serial doesn't support printing 64-bit values, so we'll cast it down to 32-bit
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}



#endif // _HEADERFILE_H    // Put this line at the end of your file.