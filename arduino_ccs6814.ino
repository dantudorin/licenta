
#define CO_port  A2
#define NH3_port A1
#define NO2_port A0

void setup() {
  Serial.begin(9600);
  
}

void loop() {

      if(Serial.available() > 0) {
          int bufferedData = Serial.read();

          float CO_level = map(analogRead(CO_port), 0, 1023, 1, 1000);
          float NH3_level = map(analogRead(NH3_port), 0, 1023, 1, 500);
          float NO2_level = map(analogRead(NO2_port), 0, 1023, 0.05, 10);
  
          String dataToSend = String(String(CO_level) + ";" + String(NH3_level) + ";" + String(NO2_level));
          
          Serial.println(dataToSend);
          
        }

        
}
//String dataToSend;

//void setup() {
//
//      Serial.begin(9600);
//    
//  }

//void loop() {
//
//    if(Serial.available() > 0) {
//        int readedValue = Serial.read();
//        dataToSend = "Iti trimit date boss";
//        Serial.println(dataToSend);
//      }
//    
//  }
