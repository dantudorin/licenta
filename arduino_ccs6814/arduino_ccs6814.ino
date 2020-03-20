#include <math.h>
#include <Arduino.h>

enum channel {
  CH_NH3, CH_RED, CH_OX
};
typedef enum channel channel_t;

enum gas {
  CO, NO2, NH3, C3H8, C4H10, CH4, H2, C2H5OH
};
typedef enum gas gas_t;

#define NH3PIN A1
#define COPIN A0
#define OXPIN A2

uint16_t NH3baseR;
uint16_t REDbaseR;
uint16_t OXbaseR;

uint16_t getResistance(channel_t channel) {
      unsigned long rs = 0;
      int counter = 0;

  switch (channel) {
    case CH_NH3:
      for(int i = 0; i < 100; i++) {
        rs += analogRead(NH3PIN);
        counter++;
        delay(2);
      }
      return rs/counter;
    case CH_RED:
      for(int i = 0; i < 100; i++) {
        rs += analogRead(COPIN);
        counter++;
        delay(2);
      }
      return rs/counter;
    case CH_OX:      
      for(int i = 0; i < 100; i++) {
        rs += analogRead(OXPIN);
        counter++;
        delay(2);
      }
      return rs/counter;

  }

  return 0;
}

void calibrateMICS() {
 
  uint8_t seconds = 10;
  
  uint8_t delta = 2;

  uint16_t bufferNH3[seconds];
  uint16_t bufferRED[seconds];
  uint16_t bufferOX[seconds];
  
  uint8_t pntrNH3 = 0;
  uint8_t pntrRED = 0;
  uint8_t pntrOX = 0;
  
  uint16_t fltSumNH3 = 0;
  uint16_t fltSumRED = 0;
  uint16_t fltSumOX = 0;

  uint16_t curNH3;
  uint16_t curRED;
  uint16_t curOX;

  bool NH3stable = false;
  bool REDstable = false;
  bool OXstable = false;

  for (int i = 0; i < seconds; ++i) {
    bufferNH3[i] = 0;
    bufferRED[i] = 0;
    bufferOX[i] = 0;
  }

  do {
    delay(1000);
    unsigned long rs = 0;
    delay(50);
    for (int i = 0; i < 3; i++) {
    delay(1);
    rs += analogRead(NH3PIN);
    }
    curNH3 = rs/3;
    rs = 0;
    delay(50);
    for (int i = 0; i < 3; i++) {
    delay(1);
    rs += analogRead(COPIN);
    }
    curRED = rs/3;
    rs = 0;
    delay(50);
    for (int i = 0; i < 3; i++) {
    delay(1);
    rs += analogRead(OXPIN);
    }
    curOX = rs/3;

    fltSumNH3 = fltSumNH3 + curNH3 - bufferNH3[pntrNH3];
    fltSumRED = fltSumRED + curRED - bufferRED[pntrRED];
    fltSumOX = fltSumOX + curOX - bufferOX[pntrOX];

    bufferNH3[pntrNH3] = curNH3;
    bufferRED[pntrRED] = curRED;
    bufferOX[pntrOX] = curOX;
    
    NH3stable = abs(fltSumNH3 / seconds - curNH3) < delta;
    REDstable = abs(fltSumRED / seconds - curRED) < delta;
    OXstable = abs(fltSumOX / seconds - curOX) < delta;

    pntrNH3 = (pntrNH3 + 1) % seconds ;
    pntrRED = (pntrRED + 1) % seconds;
    pntrOX = (pntrOX + 1) % seconds;
    
  } while (!NH3stable || !REDstable || !OXstable);

  NH3baseR = fltSumNH3 / seconds;
  REDbaseR = fltSumRED / seconds;
  OXbaseR = fltSumOX / seconds;
}

uint16_t getBaseResistance(channel_t channel) {
     switch (channel) {
       case CH_NH3:
         return NH3baseR;
       case CH_RED:
         return REDbaseR;
       case CH_OX:
         return OXbaseR;
     } 
  return 0;
}

float getCurrentRatio(channel_t channel) {
  float baseResistance = (float) getBaseResistance(channel);
  float resistance = (float) getResistance(channel);

  return resistance / baseResistance * (1023.0 - baseResistance) / (1023.0 - resistance);
  
  return -1.0;
}

float measureMICS(gas_t gas) {
  float ratio;
  float c = 0;

  switch (gas) {
    case CO:
      ratio = getCurrentRatio(CH_RED);
      c = pow(ratio, -1.179) * 4.385;
      break;
    case NO2:
      ratio = getCurrentRatio(CH_OX);
      c = pow(ratio, 1.007) / 6.855;
      break;
    case NH3:
      ratio = getCurrentRatio(CH_NH3);
      c = pow(ratio, -1.67) / 1.47;
      break;
    case C3H8:
      ratio = getCurrentRatio(CH_NH3);
      c = pow(ratio, -2.518) * 570.164;
      break;
    case C4H10:
      ratio = getCurrentRatio(CH_NH3);
      c = pow(ratio, -2.138) * 398.107;
      break;
    case CH4:
      ratio = getCurrentRatio(CH_RED);
      c = pow(ratio, -4.363) * 630.957;
      break;
    case H2:
      ratio = getCurrentRatio(CH_RED);
      c = pow(ratio, -1.8) * 0.73;
      break;
    case C2H5OH:
      ratio = getCurrentRatio(CH_RED);
      c = pow(ratio, -1.552) * 1.622;
      break;
  }

  return isnan(c) ? -1 : c;
}

void setup() {
  
  Serial.begin(9600);
  calibrateMICS();
  Serial.println("Calibration complete");
}

void loop() {

  if(Serial.available() > 0) {
          int bufferedData = Serial.read();
          String dataToSend = String(String(measureMICS(CO)) + ";" + String(measureMICS(NH3)) + ";" + String(measureMICS(NO2)));
          
          Serial.println(dataToSend);
          
        }  
 
}













//
//
//
//
//
//
//
//#define CO_port  A2
//#define NH3_port A1
//#define NO2_port A0
//
//void setup() {
//  Serial.begin(9600);
//  
//}
//
//void loop() {
//
//      float CO_level = map(analogRead(CO_port), 0, 1023, 1, 1000);
//      float NH3_level = map(analogRead(NH3_port), 0, 1023, 1, 500);
//      float NO2_level = map(analogRead(NO2_port), 0, 1023, 0.05, 10);
//      
//          
//}
