#ifndef _DACSHIELD_H    // Put these two lines at the top of your file.
#define _DACSHIELD_H    // (Use a suitable name, usually based on the file name.)

#include <CommandParser.h>  //Include the command parser library. https://github.com/Uberi/Arduino-CommandParser/tree/master
#include <math.h>  //Math library for NAN definition

#define IDN_string "Amherst College, Current DAC, SN001, A00.01.00"
#define VREF 2.5
#define RESOLUTION 16

//Set the Chip Select pin if not already defined.
#ifndef CS_PIN
#define CS_PIN 10
#endif

#define CH1_RESISTOR 50000  //RN3 on Circuit Board
#define CH2_RESISTOR 27000  //RN4 on Circuit Board
#define CH3_RESISTOR 20000  //RN1 on Circuit Board
#define CH4_RESISTOR 1000   //RN2 on Circuit Board

//DAC Defines
//DAC Output Commands
#define DAC_A 0x00
#define DAC_B 0x01
#define DAC_C 0x02
#define DAC_D 0x03
#define DAC_ALL 0x04

//DAC Output Range Commands
#define OUTPUT_RANGE_A 0x08
#define OUTPUT_RANGE_B 0x09
#define OUTPUT_RANGE_C 0x0A
#define OUTPUT_RANGE_D 0x0B
#define OUTPUT_RANGE_ALL 0x0C

//DAC Control Commands
#define NOP 0x18
#define CLEAR 0x1C
#define LOAD 0x1D
#define CONTROL_REG 0x19

//DAC Power Control Commands
#define POWER_REG 0x10
#define POWER_ON_ALL 0x000F
#define POWER_OFF_ALL 0x0000

//Defaults
#define RANGE_DEFAULT 0x0002 //Range set to +/-5V


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
void gain_polar2range(uint8_t gain, bool bipolar){
  switch(gain){
    case 2:
      Serial.println("+5");
      return;
    case 4:
      if (bipolar){
        Serial.println("+/-5");
        return;
      }
      Serial.println("+10");
      return;
    case 8:
      Serial.println("+/-10");
      return;
    default:
      Serial.println("Unknown Gain stored");
  }
}

void float2val_units(double setpoint){
  //Calculate correct units from magnitude of value and assign the variables
  if (isnan(setpoint)){  //If the value is nan, print it nicely, without units
    Serial.println("NaN");
    return;
  }
  if (abs(setpoint) > 1){
    Serial.print(setpoint, 4);
    Serial.println("A");
  }
  else if (abs(setpoint) > 1e-3){
    Serial.print(setpoint*1e3, 4);
    Serial.println("mA");
  }
  else{
    Serial.print(setpoint*1e6, 4);
    Serial.println("uA");
  }
}

void SPI_Write(uint8_t command, uint16_t  data){
  uint8_t upper_byte = (uint8_t)(data >> 8);
  uint8_t lower_byte = (uint8_t)(data & 0xFF); 
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(command);
  SPI.transfer(upper_byte);
  SPI.transfer(lower_byte);
  digitalWrite(CS_PIN, HIGH);
}

uint16_t float2bin(double inputValue, uint16_t resistance, bool bipolar, uint8_t gain){
  // convert float to the 2's complement 
  double temp = resistance*inputValue;
  temp /= VREF*gain;
  if (bipolar){
    temp += 0.5;
  }
  
  int32_t binary = round((pow(2, RESOLUTION))*temp);
  return 0x8000^(uint16_t)constrain(binary, 0, pow(2, RESOLUTION)-1);  //Return the constained values
  // XOR to invert the MSB to get 2's complement
}

double bin2float(uint16_t binary, uint16_t resistance, bool bipolar, uint8_t gain){
  //Convert 2's compliment to analog value
  // XOR to invert MSB 
  double temp = (0x8000^binary)/(pow(2,RESOLUTION));
  temp *= VREF*gain;
  
  if (bipolar){
    temp -= gain*VREF/2;
  }
  return (temp/resistance);
}

//DAC Functions
void DAC_setup(bool enable){
  SPI_Write(CLEAR, 0x0000); //Reset the outputs to zero
  SPI_Write(OUTPUT_RANGE_ALL, RANGE_DEFAULT); //Range to +/- 5V
  if (enable){
    SPI_Write(POWER_REG, POWER_ON_ALL);  // Power on all dacs
  }
}

void DAC_output(uint8_t channel, uint16_t binary){
  SPI_Write(channel,binary);
}

// Command Functions
void cmdReset(MyCommandParser::Argument *args, char *response){
  // Reset all outputs to Zero

  //Reset all internal variables for tracking settings
  Channel1Setting = 0;
  Channel2Setting = 0;
  Channel3Setting = 0;
  Channel4Setting = 0;
  Channel1Gain = 4;
  Channel2Gain = 4;
  Channel3Gain = 4;
  Channel4Gain = 4;
  Channel1Bipolar = 1;
  Channel2Bipolar = 1;
  Channel3Bipolar = 1;
  Channel4Bipolar = 1;
  Channel1Setpoint = 0.0;
  Channel2Setpoint = 0.0;
  Channel3Setpoint = 0.0;
  Channel4Setpoint = 0.0;
  DAC_setup(true);
  strlcpy(response, "OK", MyCommandParser::MAX_RESPONSE_SIZE);
}
void cmdSetChannelCurrent(MyCommandParser::Argument *args, char *response){

  uint32_t resistor;
  uint8_t gain;
  bool bipolar;
  double *setpoint_ptr = NULL;

  //Parse the input args:
  uint8_t channel = (uint8_t)args[0].asInt64;
  Serial.print("Channel: ");
  Serial.println(channel);
  double setpoint = args[1].asDouble;
  Serial.print("Setting: ");
  Serial.println(setpoint);
  char * units = args[2].asString;
  //Serial.print("Units: ");
  
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
      setpoint_ptr = &Channel1Setpoint;
      channel = DAC_A;  //re-use channel with the register mask
      break;
    case 2:
      resistor = CH2_RESISTOR;
      gain = Channel2Gain;
      bipolar = Channel2Bipolar;
      setpoint_ptr = &Channel2Setpoint;
      channel = DAC_B;
      break;
    case 3:
      resistor = CH3_RESISTOR;
      gain = Channel3Gain;
      bipolar = Channel3Bipolar;
      setpoint_ptr = &Channel3Setpoint;
      channel = DAC_C;
      break;
    case 4:
      resistor = CH4_RESISTOR;
      gain = Channel4Gain;
      bipolar = Channel4Bipolar;
      setpoint_ptr = &Channel4Setpoint;
      channel = DAC_D;
      break;
    default:
      strlcpy_PF(response, F("ERROR: Channel not found, options are 1, 2, 3, and 4"), MyCommandParser::MAX_RESPONSE_SIZE);
      return;
  }
  //Calculate DAC Command from float
  uint16_t binary = float2bin(setpoint, resistor, bipolar, gain);
  Serial.println(binary, BIN);
  //Send Commands to DAC
  DAC_output(channel, binary);
  //Report actual current
  double current = bin2float(binary, resistor, bipolar, gain);
  *setpoint_ptr = current;
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
  float2val_units(setpoint);
  strlcpy(response, "OK", MyCommandParser::MAX_RESPONSE_SIZE);
}
void cmdStatus(MyCommandParser::Argument *args, char *response){
  double current;
  char units[3];
  // Print out current settings
  Serial.println("Channel 1:");
  Serial.print("\tCurrent:\t");
  float2val_units(Channel1Setpoint);
  Serial.print("\tRange:\t\t");
  gain_polar2range(Channel1Gain,Channel1Bipolar);
  Serial.println("Channel 2:");
  Serial.print("\tCurrent:\t");
  float2val_units(Channel2Setpoint);
  Serial.print("\tRange:\t\t");
  gain_polar2range(Channel2Gain,Channel2Bipolar);
  Serial.println("Channel 3:");
  Serial.print("\tCurrent:\t");
  float2val_units(Channel3Setpoint);
  Serial.print("\tRange:\t\t");
  gain_polar2range(Channel3Gain,Channel3Bipolar);
  Serial.println("Channel 4:");
  Serial.print("\tCurrent:\t");
  float2val_units(Channel4Setpoint);
  Serial.print("\tRange:\t\t");
  gain_polar2range(Channel4Gain,Channel4Bipolar);
  

}
void cmdIdentify(MyCommandParser::Argument *args, char *response){
  // Print out IDN String
  strlcpy_PF(response, F(IDN_string), MyCommandParser::MAX_RESPONSE_SIZE);  // copy from flash memory to save space
}
void cmdScale(MyCommandParser::Argument *args, char *response){
  //Figure out what scale to change to
  uint8_t channel = (uint8_t)args[0].asInt64;
  char * range = args[1].asString;
  uint8_t * gain = NULL;  //pointer to selected gain variable
  bool * bipolar = NULL;  //pointer to selected bipolar variable
  uint8_t command = 0x00;
  uint16_t data = 0x0000; //data dword 
  switch(channel){
    case 1:
      gain = &Channel1Gain;
      bipolar = &Channel1Bipolar;
      command = 0x08;
      Channel1Setpoint = NAN;  //Invalidate the setpoint in memory since the scale changed
      break;
    case 2:
      gain = &Channel2Gain;
      bipolar = &Channel2Bipolar;
      command = 0x09;
      Channel2Setpoint = NAN;  //Invalidate the setpoint in memory since the scale changed
      break;
    case 3:
      gain = &Channel3Gain;
      bipolar = &Channel3Bipolar;
      command = 0x0A;
      Channel3Setpoint = NAN;  //Invalidate the setpoint in memory since the scale changed
      break;
    case 4:
      gain = &Channel4Gain;
      bipolar = &Channel4Bipolar;
      command = 0x0B;
      Channel4Setpoint = NAN;  //Invalidate the setpoint in memory since the scale changed
      break;
    default:
      strlcpy_PF(response, F("ERROR: Channel not found, options are 1, 2, 3, and 4"), MyCommandParser::MAX_RESPONSE_SIZE);
      return;
  }
  //Set the channel scale
  if (strcmp(range, "+5")==0){
    *gain = 2;
    *bipolar = false;
    data = 0x0000;
  }
  else if (strcmp(range, "+10")==0){
    *gain = 4;
    *bipolar = false;
    data = 0x0001;
  }
  else if (strcmp(range, "+/-5")==0){
    *gain = 4;
    *bipolar = true;
    data = 0x0003;
  }
  else if (strcmp(range, "+/-10")==0){
    *gain = 8;
    *bipolar = true;
    data = 0x0004;
  }
  else{
    strlcpy_PF(response, F("ERROR: Range not found, options are +5,+10,+/-5,+/-10"), MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  //Update the channel values
  SPI_Write(command, data);
  strlcpy(response, "OK", MyCommandParser::MAX_RESPONSE_SIZE);

}
void cmd_test(MyCommandParser::Argument *args, char *response) {
  Serial.print("string: "); Serial.println(args[0].asString);
  Serial.print("double: "); Serial.println(args[1].asDouble);
  Serial.print("int64: "); Serial.println((int32_t)args[2].asInt64); // NOTE: on older AVR-based boards, Serial doesn't support printing 64-bit values, so we'll cast it down to 32-bit
  Serial.print("uint64: "); Serial.println((uint32_t)args[3].asUInt64); // NOTE: on older AVR-based boards, Serial doesn't support printing 64-bit values, so we'll cast it down to 32-bit
  strlcpy(response, "success", MyCommandParser::MAX_RESPONSE_SIZE);
}



#endif // _HEADERFILE_H    // Put this line at the end of your file.