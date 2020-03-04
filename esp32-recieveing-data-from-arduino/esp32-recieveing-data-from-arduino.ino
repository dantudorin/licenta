//ESP32 -> recieving data from ARDUINO
#include <HardwareSerial.h>
#include "Adafruit_CCS811.h"
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <SDS011.h>


HardwareSerial MySerial(1);
HTU21D myHumidity;
Adafruit_CCS811 ccs;
SDS011 my_sds;

#ifdef ESP32
HardwareSerial port(2);
#endif

float p10, p25;
int err;
String dataSent;

float CO_level;
float NO2_level;
float NH3_level;

void setup() {
  
    Serial.begin(9600);
    
    MySerial.begin(9600, SERIAL_8N1, 14, 12); //RX, TX
    port.begin(9600,SERIAL_8N1, 16, 17);
    my_sds.begin(&port);
    
    Serial.println("HTU21D test");
    myHumidity.begin(); 

    Serial.println("CCS811 test");
    if(!ccs.begin()){
      Serial.println("Failed to start CCS811 sensor! Please check your wiring.");
      while(1);
    }

    // Wait for the sensor to be ready
    while(!ccs.available());
}

void loop() {
//  CJMCU 6814 -SENSOR 
  Serial.println("============START============="); 
  if(MySerial.available() > 0) {
       
       dataSent = MySerial.readString();
      
       int index1 = dataSent.indexOf(';');
       CO_level = dataSent.substring(0, index1).toFloat();
       
       int index2 = dataSent.indexOf(';', index1 + 1);
       NH3_level = dataSent.substring(index1 + 1, index2).toFloat();
       
       int index3 = dataSent.indexOf(';', index2 + 1);
       NO2_level = dataSent.substring(index2 + 1, index3).toFloat();

      Serial.print("CO_level : ");
      Serial.print(CO_level);
      Serial.print(" || NH3_level : ");
      Serial.print(NH3_level);
      Serial.print(" || NO2_level : ");
      Serial.println(NO2_level);

      
    }

//   SDS011 sensor
      err = my_sds.read(&p25, &p10);
      if (!err) {
        Serial.print("P2.5: " + String(p25) + "  ");
        Serial.println("P10:  " + String(p10));
      }
    
 
    //Temperature and Humidity sensor
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();

  Serial.print("Temperature:");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print(" Humidity:");
  Serial.print(humd, 1);
  Serial.println("%");

  //  CO2 -- VOC sensor
  if(ccs.available()){
    if(!ccs.readData()){
      Serial.print("CO2: ");
      Serial.print(ccs.geteCO2());
      Serial.print("ppm  || ");
      Serial.print("TVOC: ");
      Serial.print(ccs.getTVOC());
      Serial.println(" ppb");
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }
    Serial.println("========================END===========================");  
    MySerial.write(0);
    delay(500);
    
}
