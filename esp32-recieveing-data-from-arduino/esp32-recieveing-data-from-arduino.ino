//ESP32 -> recieving data from ARDUINO
#include <HardwareSerial.h>
#include "Adafruit_CCS811.h"
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <SDS011.h>
#include <ThingSpeak.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

WebServer Server;
AutoConnect Portal(Server);
WiFiClient client;

const char * apiKey = "6BMM7OIENQ8EUPMU";
const long channelNumber = 1013688;

HardwareSerial MySerial(1);
HTU21D myHumidity;
Adafruit_CCS811 ccs;
SDS011 my_sds;

#ifdef ESP32
HardwareSerial port(2);
#endif

float p10, p25;
float CO2_level, VOC_level;
int err;
String dataSent;
float humd;
float temp;

float CO_level;
float NO2_level;
float NH3_level;

//RGB LED
const int RED_COL = 26;
const int BLUE_COL = 27;
const int GREEN_COL = 25;

//RESET BUTTON
const int buttonPin = 33;

void rootPage() {
  char content[] = "ESP32 AUTOCONNECT TO WIFI";
  Server.send(200, "text/plain", content);
}
 
void setup() {
  
    Serial.begin(9600);
    delay(5000);
    
    pinMode(RED_COL, OUTPUT);
    pinMode(BLUE_COL, OUTPUT);
    pinMode(GREEN_COL, OUTPUT);
    pinMode(buttonPin, INPUT);

    digitalWrite(GREEN_COL, HIGH);
    digitalWrite(RED_COL, HIGH);
    digitalWrite(BLUE_COL, LOW);
    
    MySerial.begin(9600, SERIAL_8N1, 14, 12); //RX, TX
    port.begin(9600,SERIAL_8N1, 16, 17); //RX, TX
    my_sds.begin(&port);
  
    Serial.println("HTU21D test");
    myHumidity.begin(); 

    Serial.println("CCS811 test");
    if(!ccs.begin()){
      Serial.println("Failed to start CCS811 sensor! Please check your wiring.");
      while(1);
    }
    
    MySerial.write(0);
    delay(1000);
    if(MySerial.available() > 0) {
         MySerial.read();
         Serial.println("CJMCU 6814 is already calibrated");
      }else {
         Serial.print("CJMCU 6814 in calibration! please wait!");
              while(MySerial.available() == 0) {
                            Serial.print(".");
                            delay(3000);
              }
        Serial.println(); 
        String cjmcuMessage = MySerial.readString();
        Serial.println(cjmcuMessage);
        
      }
      digitalWrite(GREEN_COL, HIGH);
      digitalWrite(RED_COL, LOW);
      Server.on("/", rootPage);   
      if (Portal.begin()) {
          Serial.println("WiFi connected: " + WiFi.localIP().toString());
      }

    digitalWrite(RED_COL, LOW);
    digitalWrite(BLUE_COL, HIGH);
    digitalWrite(GREEN_COL, LOW); 
    
    // Wait for the sensor to be ready
    while(!ccs.available()){
       float temp = ccs.calculateTemperature();
       ccs.setTempOffset(temp - 25.0);
    }
    
    Serial.println("Connected to internet");
    
    ThingSpeak.begin(client);
}

void loop() {
  Portal.handleClient();

   if(WiFi.status() == WL_CONNECTED) {
//        Serial.println("E conectat la internet");
        
        digitalWrite(RED_COL, LOW);
        digitalWrite(BLUE_COL, HIGH);      
        
        MySerial.write(0);
        delay(100);
            if(MySerial.available() > 0) {
    
           //  CJMCU 6814 -SENSOR
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
    
          //   SDS011 sensor
    
          err = my_sds.read(&p25, &p10);
          if (!err) {
            Serial.print("P2.5: " + String(p25) + "  ");
            Serial.println("P10:  " + String(p10));
          }
    
          //   Temperature and Humidity sensor
          humd = myHumidity.readHumidity();
          temp = myHumidity.readTemperature();
    
          Serial.print("Temperature:");
          Serial.print(temp, 1);
          Serial.print("C");
          Serial.print(" Humidity:");
          Serial.print(humd, 1);
          Serial.println("%");
    
          //  CO2 -- VOC sensor
          if(ccs.available()){
            if(!ccs.readData()){
          
              CO2_level = ccs.geteCO2();
              Serial.print("CO2: ");
              Serial.print(CO2_level);
              Serial.print("ppm  || ");
              
              VOC_level = ccs.getTVOC();
              Serial.print("TVOC: ");
              Serial.print(VOC_level);
              Serial.println(" ppb");
            }
            else{
               Serial.println("ERROR!");
               while(1);
            }
        }
        
        Serial.println(uploadToThingSpeak(CO_level,NH3_level,humd,temp,p25,p10,VOC_level,CO2_level));
      }
    } else {
//        Serial.println("Nu mai e conectat la net");
        digitalWrite(RED_COL, HIGH);
        digitalWrite(BLUE_COL, LOW);
        
        if(digitalRead(buttonPin) == HIGH) {
            Serial.println("RESETTING THE BOARD!");
            ESP.restart();
          }
      }
}

String uploadToThingSpeak(float CO_level, float NH3_level, float humidity,
                              float temp, float p25, float p10, float VOC_level, float CO2_level) {
           
    ThingSpeak.setField(1, temp);
    ThingSpeak.setField(2, CO_level);
    ThingSpeak.setField(3, p25);
    ThingSpeak.setField(4, p10);
    ThingSpeak.setField(5, CO2_level);
    ThingSpeak.setField(6, VOC_level);
    ThingSpeak.setField(7, humidity);
    ThingSpeak.setField(8, NH3_level);
    
    ThingSpeak.writeFields(channelNumber, apiKey);
    delay(20000);
    
    int response = ThingSpeak.getLastReadStatus();
    
    if(response == 200) {
        return "successfully updated data to the server";
      }else {
        return "couldn't update data to the server";  
      }                              
}
