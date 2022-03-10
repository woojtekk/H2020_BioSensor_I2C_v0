#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
#include <avr/wdt.h>
#include <CRC32.h>
#include <avr/wdt.h>

#define DEBUG  true
#define Serial if(DEBUG)Serial

#define I2C_ADDRESS       0x66
#define I2C_SCL           13              // A5  - those pins can not be changed for arduino uno
#define I2C_SDA           12              // A4  - those pins can not be changed for arduino uno
#define PIN_ALARM         23
#define BIOSENSOR_ID      UINT32_MAX      // 24 bits unsigned integer. range: 0 - 16`777`215 

#define REG_ID                      0x01  // uint32 ==> 4 byte  R
#define REG_RESET                   0x02  // uint8  ==> 1 byte  W
#define REG_STATUS                  0x03  // uint8  ==> 1 byte  R
#define REG_ERROR                   0x04  // uint8  ==> 1 byte  R
#define REG_LIFETIME                0x05  // uint32 ==> 4 byte  R
#define REG_INTERNAL_TEMPERATURE    0x06  // int32  ==> 4 byte  R
#define REG_INTERNAL_PRESSURE       0x07  // uint32 ==> 4 byte  R
#define REG_INTERNAL_FLOW           0x08  // uint32 ==> 4 byte  R
#define REG_ALARM                   0x09  // uint8  ==> 1 byte  R
#define REG_MEASURE_TIME_INTERWAL   0x0A  // uint32 ==> 4 byte  R/W
#define REG_DETECTION_TRESHOLD      0x0B  // uint32 ==> 4 byte  R/W
#define REG_DETECTION_VALUE         0x0C  // uint32 ==> 4 byte  R/W
#define REG_ONOFF_CRC               0x0D  // uint32 ==> 4 byte  R/W

struct sensorData_t{
  uint32_t ID;                              // indyvidual ID of the device
  uint32_t LIFETIME;                        // time since start in seconds
  uint32_t MEASURE_TIME_INTERWAL;           // time to collect sample for single measurements
  uint32_t LAST_UPDATE;                     // time since last status update in seconds  

  int32_t  INTERNAL_TEMPERATURE;            /* temperatur:     36.660*C *1000 = 36660   
                                               internal temperature of sample medium just for debuging and services */
  int32_t  INTERNAL_PRESSURE;               /* pressure:   1013.450hPa * 1000 = 1013450 
                                               internal pressure of sample medium just for debuging and services */
  int32_t  INTERNAL_FLOW;                   /* flow:        123.456 ml/min * 1000 = 123456  
                                               internal flow of sample medium just for debuging and services */
  int32_t  DETECTION_TRESHOLD;              /* Detection treshold:  12345678 - at the moement not sure about value  
                                               detection treshold. above this value ALARM wil be reported */
  int32_t  DETECTION_VALUE;                 /* Detection value: 12345678 - at the moement not sure about value  
                                               real value of measurements just for debuging and services */

  uint8_t  STATUS;    /* Dev. Status. bit 0:  0 - device off or error or 1 - Normal operating mode 
                                      bit 1-7  converted to uint8. Values from 0 - 128 statsu details slist will be provided later */
  uint8_t  ERROR;     /* Dev. ERROR. bit 0:  0 - no error or 1 - some error 
                                      bit 1-7  converted to uint8. Values from 0 - 128 error details list will be provided later */
  uint8_t  ALARM;     /* Dev. ALARM. bit 0:  0 - no alarm or 1 - alarm. device reach value higher than treshold 
                                      bit 1-7  converted to uint8. Values from 0 - 128 alarm details list will be provided later */
  uint8_t  RESET;     /* Dev. RESET. bit 0-7: vonverted to uint8. indyvidual value mens reset of indyvidual component or full devices. 
                                      only for servcies, debiging, or is case of freez*/

};


sensorData_t SENSOR;
CRC32 crc32;
byte reg;
boolean IFCRC = false;

uint32_t bigNum = 16777210;
uint32_t bigNum2;

byte myArray[4];

void INIT(){          // prepare init values 
  Serial.println(":: INIT PARAM..."); 
  SENSOR.ID                     = UINT32_MAX;  
  SENSOR.RESET                  = 0;
  SENSOR.STATUS                 = 125;
  SENSOR.ALARM                  = 0;
  SENSOR.ERROR                  = 0;
  SENSOR.LIFETIME               = 0;
  SENSOR.INTERNAL_TEMPERATURE   = 1234567890;
  SENSOR.INTERNAL_PRESSURE      = 1234567890;
  SENSOR.INTERNAL_FLOW          = 1234567890;
  SENSOR.MEASURE_TIME_INTERWAL  = 1234567890;
  SENSOR.DETECTION_TRESHOLD     = 1234567890;
  SENSOR.DETECTION_VALUE        = 1234567890;

}

void INIT_RANDOM(){   // apply random values just for tests
  Serial.println(":: RANDOM INIT PARAM..."); 
  // SENSOR.ID                     = UINT32_MAX;  
  // SENSOR.RESET                  = 66;
  SENSOR.STATUS                 = random(1,256);
  SENSOR.ALARM                  = random(1,256);
  SENSOR.ERROR                  = random(256);
  // SENSOR.LIFETIME               = random(UINT32_MAX);
  SENSOR.INTERNAL_TEMPERATURE   = random(INT32_MAX);
  SENSOR.INTERNAL_PRESSURE      = random(UINT32_MAX);
  SENSOR.INTERNAL_FLOW          = random(UINT32_MAX);
  SENSOR.MEASURE_TIME_INTERWAL  = random(INT32_MAX);
  SENSOR.DETECTION_TRESHOLD     = random(UINT32_MAX);
  SENSOR.DETECTION_VALUE        = random(UINT32_MAX);

}

void softwareReset( uint8_t prescaller=WDTO_60MS) {
  // start watchdog with the provided prescaller
  wdt_enable( prescaller);
  // wait for the prescaller time to expire
  // without sending the reset signal by using
  // the wdt_reset() method
  while(1) {}
}
void  DATAPROCESS(){
  if(SENSOR.RESET == 128)  digitalWrite( 7, LOW);   // perform HardReset
  if(bitRead(SENSOR.ALARM,0) == 1) {
    Serial.println(">>>>>>>>>>>>>>>>>>> ALARM <<<<<<<<<<<<<<<<<<<");
    digitalWrite(PIN_ALARM, HIGH);
  } else {
    Serial.println("<<<<<<<<<<<<<<<<<<< NO ALARM >>>>>>>>>>>>>>>>>>>");
    digitalWrite(PIN_ALARM, HIGH);
    
  }

}

void SaveToRegister(byte reg, int howMany){
    Serial.print  (F(":: #### Save:    Register: ")); 
    Serial.print  (reg);
    Serial.print  (F(" \t :: "));
    Serial.print  (howMany);
    Serial.print  (F(" bytes :: "));

    switch(reg){
      /*========================
      RESET: 0x02 [WRITE ONLY] 
      unsigned 8 bit integer [uint8_t]
      Read: 1 byte of data
      ========================*/
      case REG_RESET:
        // Serial.print  (F("REG_RESET: "));
        if( howMany == 2 and Wire.available() ){
          byte read = Wire.read();
          Serial.print(SENSOR.ERROR);
          Serial.print(" >> ");
          SENSOR.ERROR = read;
          Serial.println(SENSOR.ERROR);
        }
        else{     
          Serial.println(":: ERR_04");
          Wire.write("ERR_04");
        }
        Serial.println(" ");
        break;

      /*========================
      MEASURE_TIME_INTERWAL: 0x0A [R/W] 
      unsigned 32 bit integer [uint32_t]
      Read: 4 byte of data
      ========================*/
      case REG_MEASURE_TIME_INTERWAL:
        // Serial.print  (F("MEASURE_TIME_INTERWAL: "));
        if( howMany == 5 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          Serial.print(SENSOR.MEASURE_TIME_INTERWAL);
          Serial.print(F(" >> "));
          SENSOR.MEASURE_TIME_INTERWAL = bigNum;
          Serial.println(SENSOR.MEASURE_TIME_INTERWAL);
        }
        else{     
          Serial.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        Serial.println(F(" "));
        break;


     /*========================
      DETECTION_TRESHOLD: 0x0A [R/W] 
      unsigned 32 bit integer [uint32_t]
      Read: 4 byte of data
      ========================*/
      case REG_DETECTION_TRESHOLD:
        // Serial.print  (F("DETECTION_TRESHOLD: "));
        if( howMany == 5 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          Serial.print(SENSOR.DETECTION_TRESHOLD);
          Serial.print(F(" >> "));
          SENSOR.DETECTION_TRESHOLD = bigNum;
          Serial.println(SENSOR.DETECTION_TRESHOLD);
        }
        else{     
          Serial.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        Serial.println(" ");
        break;

     /*========================
      DETECTION_VALUE: 0x0A [R/W] 
      unsigned 32 bit integer [uint32_t]
      Read: 4 byte of data
      ========================*/
      case REG_DETECTION_VALUE:
        // Serial.print  (F("DETECTION_VALUE: "));
        if( howMany == 5 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          Serial.print(SENSOR.DETECTION_VALUE);
          Serial.print(F(" >> "));
          SENSOR.DETECTION_VALUE = bigNum;
          Serial.println(SENSOR.DETECTION_VALUE);
        }
        else{     
          Serial.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        Serial.println(F(" "));
        break;



      default :
        Serial.println(F(":: >> EMPTY <<"));
        Wire.write("ERR_03");
        break;
    }
}

// function that executes whenever data is received from master
void receiveEvent(int howMany) {
  reg = Wire.read();
  Serial.print  (":: <<<< Receive: Register: "); 
  Serial.print  (reg);
  Serial.print  (" \t :: ");
  Serial.print  (howMany);
  Serial.println(" bytes");


  if (howMany > 1) SaveToRegister(reg,howMany); 
}


// function that executes whenever data is requested from master
void requestEvent() {
  Serial.print  (":: >>>> Send:    Register: "); 
  Serial.print  (reg);

  byte myArray[4];

  if (reg >= 0){
    switch(reg){
      /*========================
      Device ID: 0x01 [READ ONLY] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_ID:
        Serial.print  (F(" \t :: 4 bytes :: "));
        Serial.print  (SENSOR.ID);
        Serial.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.ID >> 24) & 0xFF;
        myArray[1] = (SENSOR.ID >> 16) & 0xFF;
        myArray[2] = (SENSOR.ID >>  8) & 0xFF;
        myArray[3] =  SENSOR.ID & 0xFF;

        for(int x = 0; x <= 3; x++){
          Serial.print(" ");
          Serial.print(myArray[x]);
        }
      
        Wire.write(myArray, 4);
        Serial.println(" ");
        // Serial.print("\n:: CRC::");
        // Serial.print(CRC32::calculate(myArray, 4));
        // Serial.print (" :: ");
        // Serial.println(sizeof(CRC32::calculate(myArray, 4)));

  
        // if(IFCRC){
        //   Serial.println(CRC32::calculate(myArray, 4));
        //   Wire.write(CRC32::calculate(myArray, 4));
        // }
      break;

      /*========================
      Device STATUS: 0x03 [READ ONLY] 
      unsigned 8 bit integer [uint8_t]
      SEND: 1 BYTE:
      BIT   0: 1 or 0 where 1 - normal operating mode 
                            0 - not working.
      BIT 1-7: status of the device. 
              Details will be provide later 
              Value from 0 up to 128
      ========================*/
      case REG_STATUS:
        Serial.print  (F(" \t :: 1 byte  :: "));
        Serial.print  (SENSOR.STATUS);
        Serial.print  (F(" :: in Byte: "));


        Serial.println(SENSOR.STATUS);
        Wire.write(SENSOR.STATUS);
        break;

      /*========================
      Device ERROR: 0x03 [READ ONLY] 
      unsigned 8 bit integer [uint8_t]
      SEND: 1 BYTE:
      BIT   0: 1 or 0 where 1 - ERROR occurre  
                            0 - NBO ERROR.
      BIT 1-7: ERROR LIST of the device. 
              Details will be provide later 
              Value from 0 up to 128
      ========================*/
      case REG_ERROR:
        Serial.print  (F(" \t :: 1 byte  :: "));
        Serial.print  (SENSOR.ERROR);
        Serial.print  (F(" :: in Byte: "));

        Serial.println(SENSOR.ERROR);
        Wire.write(SENSOR.ERROR);
        break;

      /*========================
      Device LIFETIME: 0x04 [READ ONLY] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_LIFETIME:
        Serial.print  (F(" \t :: 4 bytes :: "));
        Serial.print  (SENSOR.LIFETIME);
        Serial.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.LIFETIME >> 24) & 0xFF;
        myArray[1] = (SENSOR.LIFETIME >> 16) & 0xFF;
        myArray[2] = (SENSOR.LIFETIME >>  8) & 0xFF;
        myArray[3] =  SENSOR.LIFETIME & 0xFF;

        for(int x = 0; x <= 3; x++){
          Serial.print(" ");
          Serial.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        Serial.println(" ");

      break;

      /*========================
      Device INTERNAL_TEMPERATURE: 0x05 [READ ONLY] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_INTERNAL_TEMPERATURE:
        Serial.print  (F(" \t :: 4 bytes :: "));
        Serial.print  (SENSOR.INTERNAL_TEMPERATURE);
        Serial.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.INTERNAL_TEMPERATURE >> 24) & 0xFF;
        myArray[1] = (SENSOR.INTERNAL_TEMPERATURE >> 16) & 0xFF;
        myArray[2] = (SENSOR.INTERNAL_TEMPERATURE >>  8) & 0xFF;
        myArray[3] =  SENSOR.INTERNAL_TEMPERATURE & 0xFF;

        for(int x = 0; x <= 3; x++){
          Serial.print(" ");
          Serial.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        Serial.println(" ");
      break;

      /*========================
      Device INTERNAL_PRESSURE: 0x06 [READ ONLY] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_INTERNAL_PRESSURE:
        Serial.print  (F(" \t :: 4 bytes :: "));
        Serial.print  (SENSOR.INTERNAL_PRESSURE);
        Serial.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.INTERNAL_PRESSURE >> 24) & 0xFF;
        myArray[1] = (SENSOR.INTERNAL_PRESSURE >> 16) & 0xFF;
        myArray[2] = (SENSOR.INTERNAL_PRESSURE >>  8) & 0xFF;
        myArray[3] =  SENSOR.INTERNAL_PRESSURE & 0xFF;

        for(int x = 0; x <= 3; x++){
          Serial.print(" ");
          Serial.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        Serial.println(" ");

      break;

      /*========================
      Device INTERNAL_FLOW: 0x07 [READ ONLY] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/

      case REG_INTERNAL_FLOW:
        Serial.print  (F(" \t :: 4 bytes :: "));
        Serial.print  (SENSOR.INTERNAL_FLOW);
        Serial.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.INTERNAL_FLOW >> 24) & 0xFF;
        myArray[1] = (SENSOR.INTERNAL_FLOW >> 16) & 0xFF;
        myArray[2] = (SENSOR.INTERNAL_FLOW >>  8) & 0xFF;
        myArray[3] =  SENSOR.INTERNAL_FLOW & 0xFF;

        for(int x = 0; x <= 3; x++){
          Serial.print(" ");
          Serial.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        Serial.println(" ");

      break;


      /*========================
      Device ALARM: 0x08 [READ ONLY] 
      unsigned 8 bit integer [uint8_t]
      SEND: 1 BYTE:
      BIT   0: 1 or 0 where 1 - ALARM occurre  
                            0 - NO ALARM.
      BIT 1-7: ALARM LIST of the device. 
              Details will be provide later 
              Value from 0 up to 128
      ========================*/
      case REG_ALARM:
        Serial.print  (F(" \t :: 1 byte  :: "));
        Serial.print  (SENSOR.ALARM);
        Serial.print  (F(" :: in Byte: "));

        Serial.println(SENSOR.ALARM);
        Wire.write(SENSOR.ALARM);
        break;


      /*========================
      Device MEASURE_TIME_INTERWAL: 0x09 [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_MEASURE_TIME_INTERWAL:
        Serial.print  (F(" \t :: 4 bytes :: "));
        Serial.print  (SENSOR.MEASURE_TIME_INTERWAL);
        Serial.print  (F(" :: in Byte: "));


        myArray[0] = (SENSOR.MEASURE_TIME_INTERWAL >> 24) & 0xFF;
        myArray[1] = (SENSOR.MEASURE_TIME_INTERWAL >> 16) & 0xFF;
        myArray[2] = (SENSOR.MEASURE_TIME_INTERWAL >>  8) & 0xFF;
        myArray[3] =  SENSOR.MEASURE_TIME_INTERWAL & 0xFF;

        for(int x = 0; x <= 3; x++){
          Serial.print(" ");
          Serial.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        Serial.println(" ");
      break;


    /*========================
      Device DETECTION_TRESHOLD: 0x0A [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_DETECTION_TRESHOLD:
        Serial.print  (F(" \t :: 4 bytes :: "));
        Serial.print  (SENSOR.DETECTION_TRESHOLD);
        Serial.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.DETECTION_TRESHOLD >> 24) & 0xFF;
        myArray[1] = (SENSOR.DETECTION_TRESHOLD >> 16) & 0xFF;
        myArray[2] = (SENSOR.DETECTION_TRESHOLD >>  8) & 0xFF;
        myArray[3] =  SENSOR.DETECTION_TRESHOLD & 0xFF;

        for(int x = 0; x <= 3; x++){
          Serial.print(" ");
          Serial.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        Serial.println(" ");
      break;


    /*========================
      Device DETECTION_VALUE: 0x0B [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_DETECTION_VALUE:
        Serial.print  (F(" \t :: 4 bytes :: "));
        Serial.print  (SENSOR.DETECTION_VALUE);
        Serial.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.DETECTION_VALUE >> 24) & 0xFF;
        myArray[1] = (SENSOR.DETECTION_VALUE >> 16) & 0xFF;
        myArray[2] = (SENSOR.DETECTION_VALUE >>  8) & 0xFF;
        myArray[3] =  SENSOR.DETECTION_VALUE & 0xFF;

        for(int x = 0; x <= 3; x++){
          Serial.print(" ");
          Serial.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        Serial.println(" ");
      break;

      default :
        Serial.println(F(":: >> EMPTY <<"));
        Wire.write("ERR_02");
        break;
    }
  } // end if( reg >= 0)
  else{
    Serial.print(":: ERR Wrong Register:");
    Serial.println(reg);
    Wire.write("ERR_01");
  }

    reg = -1;     // if trransmistion go well then set regster for unreal value to prewent resend the same information 
    Serial.println("");
}

void INIT_I2C(){
  Serial.println("INIT I2C:");
  Serial.print("I2C address: ");  
  Serial.print((I2C_ADDRESS));

  Wire.begin(I2C_ADDRESS);      /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
}


void setup() {
  randomSeed(analogRead(0));
  pinMode( PIN_ALARM , OUTPUT);
  digitalWrite( PIN_ALARM, HIGH);

  Serial.begin(9600);
  Serial.println(" ");
  Serial.println("======== SETUP ======== ");
  INIT();
  INIT_I2C();
  
}
  byte a =0;
  uint32_t last = 0;

void loop() {
  int dt =0;
  dt = (millis() - last);
  Serial.print("ms: ");        Serial.print(dt);
  Serial.print(" \t :: Alarm : ");  Serial.print(bitRead(SENSOR.ALARM,0)); 
  Serial.print(" [");  Serial.print((uint8_t)(SENSOR.ALARM >> 1)); Serial.print("]");
 
  Serial.print(" \t :: Status: ");  Serial.print(bitRead(SENSOR.STATUS,0));
  Serial.print(" [");  Serial.print((uint8_t)(SENSOR.STATUS >> 1)); Serial.print("]");

  Serial.print(" \t :: ERROR: ");  Serial.print(bitRead(SENSOR.ERROR,0));
  Serial.print(" [");  Serial.print((uint8_t)(SENSOR.ERROR >> 1)); Serial.print("]");

  Serial.print(" \t :: RESET: ");  Serial.println(SENSOR.STATUS);



  if ( dt > 20000 ) {
    last = millis();
    INIT_RANDOM(); 
    DATAPROCESS();
  }
  SENSOR.LIFETIME = millis()/1000;
  delay(1000);

}
