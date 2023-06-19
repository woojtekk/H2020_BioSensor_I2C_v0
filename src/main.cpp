#include <Arduino.h>
#include <Wire.h>
#include "main.h"
#include <stdint.h>


#define DEBUG  true
#define DEBUG_PRINT if(DEBUG) Serial



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

  uint32_t TIMESTAMP;
  uint32_t ID_CHIP;
  uint32_t FW;

  uint16_t IP1, IP2,IP3,IP4,IP5;


};


sensorData_t SENSOR;
byte reg;
boolean IFCRC = false;

uint32_t bigNum = 16777210;
uint32_t bigNum2;

byte myArray[4];

void INIT(){          // prepare init values 
  DEBUG_PRINT.println(":: INIT PARAM..."); 
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

  SENSOR.TIMESTAMP = 0;
  SENSOR.ID_CHIP = 98765432;
  SENSOR.FW = 234432;
  SENSOR.IP1 = 1111;
  SENSOR.IP2 = 2222;
  SENSOR.IP3 = 3333;
  SENSOR.IP4 = 4444;
  SENSOR.IP5 = 5555;

}

void INIT_RANDOM(){   // apply random values just for tests
  DEBUG_PRINT.println(":: RANDOM INIT PARAM..."); 
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

void softwareReset() {
  esp_restart();
  while(1) {}
}
void  DATAPROCESS(){
  if(SENSOR.RESET == 128)  ESP.restart();   // perform HardReset
  if(bitRead(SENSOR.ALARM,0) == 1) {
    DEBUG_PRINT.println(">>>>>>>>>>>>>>>>>>> ALARM <<<<<<<<<<<<<<<<<<<");
    digitalWrite(PIN_ALARM, HIGH);
  } else {
    DEBUG_PRINT.println("<<<<<<<<<<<<<<<<<<< NO ALARM >>>>>>>>>>>>>>>>>>>");
    digitalWrite(PIN_ALARM, HIGH);
    
  }

}



void SaveToRegister(byte reg, int howMany){
    DEBUG_PRINT.print  (F(":: #### Save:    Register: ")); 
    DEBUG_PRINT.print  (reg);
    DEBUG_PRINT.print  (F(" \t :: "));
    DEBUG_PRINT.print  (howMany);
    DEBUG_PRINT.print  (F(" bytes :: "));

    switch(reg){
      /*========================
      RESET: 0x02 [WRITE ONLY] 
      unsigned 8 bit integer [uint8_t]
      Read: 1 byte of data
      ========================*/
      case REG_RESET:
        // DEBUG_PRINT.print  (F("REG_RESET: "));
        if( howMany == 2 and Wire.available() ){
          byte read = Wire.read();
          DEBUG_PRINT.print(SENSOR.ERROR);
          DEBUG_PRINT.print(" >> ");
          SENSOR.ERROR = read;
          DEBUG_PRINT.println(SENSOR.ERROR);
        }
        else{     
          DEBUG_PRINT.println(":: ERR_04");
          Wire.write("ERR_04");
        }
        DEBUG_PRINT.println(" ");
        break;

      // /*========================
      // MEASURE_TIME_INTERWAL: 0x0A [R/W] 
      // unsigned 32 bit integer [uint32_t]
      // Read: 4 byte of data
      // ========================*/
      // case REG_MEASURE_TIME_INTERWAL:
      //   // DEBUG_PRINT.print  (F("MEASURE_TIME_INTERWAL: "));
      //   if( howMany == 5 and Wire.available() ){

      //     bigNum = 0;
      //     bigNum = Wire.read();
      //     while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

      //     DEBUG_PRINT.print(SENSOR.MEASURE_TIME_INTERWAL);
      //     DEBUG_PRINT.print(F(" >> "));
      //     SENSOR.MEASURE_TIME_INTERWAL = bigNum;
      //     DEBUG_PRINT.println(SENSOR.MEASURE_TIME_INTERWAL);
      //   }
      //   else{     
      //     DEBUG_PRINT.println(F(":: ERR_04"));
      //     Wire.write("ERR_04");
      //   }
      //   DEBUG_PRINT.println(F(" "));
      //   break;


     /*========================
      DETECTION_TRESHOLD: 0x0A [R/W] 
      unsigned 32 bit integer [uint32_t]
      Read: 4 byte of data
      ========================*/
      case REG_DETECTION_TRESHOLD:
        // DEBUG_PRINT.print  (F("DETECTION_TRESHOLD: "));
        if( howMany == 5 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          DEBUG_PRINT.print(SENSOR.DETECTION_TRESHOLD);
          DEBUG_PRINT.print(F(" >> "));
          SENSOR.DETECTION_TRESHOLD = bigNum;
          DEBUG_PRINT.println(SENSOR.DETECTION_TRESHOLD);
        }
        else{     
          DEBUG_PRINT.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        DEBUG_PRINT.println(" ");
        break;

     /*========================
      DETECTION_VALUE: 0x0A [R/W] 
      unsigned 32 bit integer [uint32_t]
      Read: 4 byte of data
      ========================*/
      case REG_TIMESTAMP:
        // DEBUG_PRINT.print  (F("DETECTION_VALUE: "));
        if( howMany == 5 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          DEBUG_PRINT.print(SENSOR.TIMESTAMP);
          DEBUG_PRINT.print(F(" >> "));
          SENSOR.TIMESTAMP = bigNum;
          DEBUG_PRINT.println(SENSOR.TIMESTAMP);
        }
        else{     
          DEBUG_PRINT.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        DEBUG_PRINT.println(F(" "));
        break;

      case REG_IP_1:
        // DEBUG_PRINT.print  (F("DETECTION_VALUE: "));
        if( howMany == 3 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          DEBUG_PRINT.print(SENSOR.IP1);
          DEBUG_PRINT.print(F(" >> "));
          SENSOR.IP1 = bigNum;
          DEBUG_PRINT.println(SENSOR.IP1);
        }
        else{     
          DEBUG_PRINT.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        DEBUG_PRINT.println(F(" "));
        break;

      case REG_IP_2:
        // DEBUG_PRINT.print  (F("DETECTION_VALUE: "));
        if( howMany == 3 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          DEBUG_PRINT.print(SENSOR.IP2);
          DEBUG_PRINT.print(F(" >> "));
          SENSOR.IP2 = bigNum;
          DEBUG_PRINT.println(SENSOR.IP2);
        }
        else{     
          DEBUG_PRINT.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        DEBUG_PRINT.println(F(" "));
        break;


      case REG_IP_3:
        // DEBUG_PRINT.print  (F("DETECTION_VALUE: "));
        if( howMany == 3 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          DEBUG_PRINT.print(SENSOR.IP3);
          DEBUG_PRINT.print(F(" >> "));
          SENSOR.IP3 = bigNum;
          DEBUG_PRINT.println(SENSOR.IP3);
        }
        else{     
          DEBUG_PRINT.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        DEBUG_PRINT.println(F(" "));
        break;


      case REG_IP_4:
        // DEBUG_PRINT.print  (F("DETECTION_VALUE: "));
        if( howMany == 3 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          DEBUG_PRINT.print(SENSOR.IP4);
          DEBUG_PRINT.print(F(" >> "));
          SENSOR.IP4 = bigNum;
          DEBUG_PRINT.println(SENSOR.IP4);
        }
        else{     
          DEBUG_PRINT.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        DEBUG_PRINT.println(F(" "));
        break;


      case REG_IP_5:
        // DEBUG_PRINT.print  (F("DETECTION_VALUE: "));
        if( howMany == 3 and Wire.available() ){

          bigNum = 0;
          bigNum = Wire.read();
          while ( Wire.available() )  bigNum = (bigNum << 8) | Wire.read();

          DEBUG_PRINT.print(SENSOR.IP5);
          DEBUG_PRINT.print(F(" >> "));
          SENSOR.IP5 = bigNum;
          DEBUG_PRINT.println(SENSOR.IP5);
        }
        else{     
          DEBUG_PRINT.println(F(":: ERR_04"));
          Wire.write("ERR_04");
        }
        DEBUG_PRINT.println(F(" "));
        break;



      default :
        DEBUG_PRINT.println(F(":: >> EMPTY <<"));
        Wire.write("ERR_03");
        break;
    }
}

// function executes whenever data is received from master
void receiveEvent(int howMany) {
  byte reg = Wire.read();
  DEBUG_PRINT.print  (":: <<<< Receive: Register: "); 
  DEBUG_PRINT.print  (reg);
  DEBUG_PRINT.print  (" \t :: ");
  DEBUG_PRINT.print  (howMany);
  DEBUG_PRINT.println(" bytes");


  if (howMany > 1) SaveToRegister(reg,howMany); 
}


// function that executes whenever data is requested from master
void requestEvent() {
  DEBUG_PRINT.print  (":: >>>> Send:    Register: "); 
  DEBUG_PRINT.print  (reg);

  byte myArray[4];
  byte myArray16[2];

  if (reg >= 0){
    switch(reg){
      /*========================
      Device ID: 0x01 [READ ONLY] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_ID:
        DEBUG_PRINT.print  (F(" \t :: 4 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.ID);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.ID >> 24) & 0xFF;
        myArray[1] = (SENSOR.ID >> 16) & 0xFF;
        myArray[2] = (SENSOR.ID >>  8) & 0xFF;
        myArray[3] =  SENSOR.ID & 0xFF;

        for(int x = 0; x <= 3; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray[x]);
        }
      
        Wire.write(myArray, 4);
        DEBUG_PRINT.println(" ");
        // DEBUG_PRINT.print("\n:: CRC::");
        // DEBUG_PRINT.print(CRC32::calculate(myArray, 4));
        // DEBUG_PRINT.print (" :: ");
        // DEBUG_PRINT.println(sizeof(CRC32::calculate(myArray, 4)));

  
        // if(IFCRC){
        //   DEBUG_PRINT.println(CRC32::calculate(myArray, 4));
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
        DEBUG_PRINT.print  (F(" \t :: 1 byte  :: "));
        DEBUG_PRINT.print  (SENSOR.STATUS);
        DEBUG_PRINT.print  (F(" :: in Byte: "));


        DEBUG_PRINT.println(SENSOR.STATUS);
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
        DEBUG_PRINT.print  (F(" \t :: 1 byte  :: "));
        DEBUG_PRINT.print  (SENSOR.ERROR);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        DEBUG_PRINT.println(SENSOR.ERROR);
        Wire.write(SENSOR.ERROR);
        break;

      /*========================
      Device INTERNAL PARAM 1: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_1:
        DEBUG_PRINT.print  (F(" \t :: 4 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.IP1);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        // myArray16[0] = (SENSOR.IP1 >> 24) & 0xFF;
        // myArray16[1] = (SENSOR.IP1 >> 16) & 0xFF;
        myArray16[0] = (SENSOR.IP1 >>  8) & 0xFF;
        myArray16[1] =  SENSOR.IP1 & 0xFF;

        for(int x = 0; x <= 1; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray16[x]);
        }
        
        Wire.write(myArray16, 2);
        DEBUG_PRINT.println(" ");

      break;

      /*========================
      Device INTERNAL PARAM 2: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_2:
        DEBUG_PRINT.print  (F(" \t :: 2 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.IP2);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        // myArray16[0] = (SENSOR.IP1 >> 24) & 0xFF;
        // myArray16[1] = (SENSOR.IP1 >> 16) & 0xFF;
        myArray16[0] = (SENSOR.IP2 >>  8) & 0xFF;
        myArray16[1] =  SENSOR.IP2 & 0xFF;

        for(int x = 0; x <= 1; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray16[x]);
        }
        
        Wire.write(myArray16, 2);
        DEBUG_PRINT.println(" ");

      break;


      /*========================
      Device INTERNAL PARAM 3: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_3:
        DEBUG_PRINT.print  (F(" \t :: 2 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.IP3);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        // myArray16[0] = (SENSOR.IP1 >> 24) & 0xFF;
        // myArray16[1] = (SENSOR.IP1 >> 16) & 0xFF;
        myArray16[0] = (SENSOR.IP3 >>  8) & 0xFF;
        myArray16[1] =  SENSOR.IP3 & 0xFF;

        for(int x = 0; x <= 1; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray16[x]);
        }
        
        Wire.write(myArray16, 2);
        DEBUG_PRINT.println(" ");

      break;


      /*========================
      Device INTERNAL PARAM 4: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_4:
        DEBUG_PRINT.print  (F(" \t :: 2 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.IP4);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        // myArray16[0] = (SENSOR.IP1 >> 24) & 0xFF;
        // myArray16[1] = (SENSOR.IP1 >> 16) & 0xFF;
        myArray16[0] = (SENSOR.IP4 >>  8) & 0xFF;
        myArray16[1] =  SENSOR.IP4 & 0xFF;

        for(int x = 0; x <= 1; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray16[x]);
        }
        
        Wire.write(myArray16, 2);
        DEBUG_PRINT.println(" ");

      break;


      /*========================
      Device INTERNAL PARAM 5: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_5:
        DEBUG_PRINT.print  (F(" \t :: 2 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.IP5);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        // myArray16[0] = (SENSOR.IP1 >> 24) & 0xFF;
        // myArray16[1] = (SENSOR.IP1 >> 16) & 0xFF;
        myArray16[0] = (SENSOR.IP5 >>  8) & 0xFF;
        myArray16[1] =  SENSOR.IP5 & 0xFF;

        for(int x = 0; x <= 1; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray16[x]);
        }
        
        Wire.write(myArray16, 2);
        DEBUG_PRINT.println(" ");

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
        DEBUG_PRINT.print  (F(" \t :: 1 byte  :: "));
        DEBUG_PRINT.print  (SENSOR.ALARM);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        DEBUG_PRINT.println(SENSOR.ALARM);
        Wire.write(SENSOR.ALARM);
        break;


    /*========================
      Device DETECTION_TRESHOLD: 0x0A [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_DETECTION_TRESHOLD:
        DEBUG_PRINT.print  (F(" \t :: 4 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.DETECTION_TRESHOLD);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.DETECTION_TRESHOLD >> 24) & 0xFF;
        myArray[1] = (SENSOR.DETECTION_TRESHOLD >> 16) & 0xFF;
        myArray[2] = (SENSOR.DETECTION_TRESHOLD >>  8) & 0xFF;
        myArray[3] =  SENSOR.DETECTION_TRESHOLD & 0xFF;

        for(int x = 0; x <= 3; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        DEBUG_PRINT.println(" ");
      break;


    /*========================
      Device DETECTION_VALUE: 0x0B [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_DETECTION_VALUE:
        DEBUG_PRINT.print  (F(" \t :: 4 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.DETECTION_VALUE);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.DETECTION_VALUE >> 24) & 0xFF;
        myArray[1] = (SENSOR.DETECTION_VALUE >> 16) & 0xFF;
        myArray[2] = (SENSOR.DETECTION_VALUE >>  8) & 0xFF;
        myArray[3] =  SENSOR.DETECTION_VALUE & 0xFF;

        for(int x = 0; x <= 3; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        DEBUG_PRINT.println(" ");
      break;


      /*========================
      Device FW: 0x0D [ READ  ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_FW:
        DEBUG_PRINT.print  (F(" \t :: 4 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.FW);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.FW >> 24) & 0xFF;
        myArray[1] = (SENSOR.FW >> 16) & 0xFF;
        myArray[2] = (SENSOR.FW >>  8) & 0xFF;
        myArray[3] =  SENSOR.FW & 0xFF;

        for(int x = 0; x <= 3; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        DEBUG_PRINT.println(" ");
      break;

      /*========================
      Device ID_CHIP: 0x0E [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_ID_CHIP:
        DEBUG_PRINT.print  (F(" \t :: 4 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.ID_CHIP);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.ID_CHIP >> 24) & 0xFF;
        myArray[1] = (SENSOR.ID_CHIP >> 16) & 0xFF;
        myArray[2] = (SENSOR.ID_CHIP >>  8) & 0xFF;
        myArray[3] =  SENSOR.ID_CHIP & 0xFF;

        for(int x = 0; x <= 3; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        DEBUG_PRINT.println(" ");
      break;


      /*========================
      Device REG_TIMESTAMP: 0x0E [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_TIMESTAMP:
        DEBUG_PRINT.print  (F(" \t :: 4 bytes :: "));
        DEBUG_PRINT.print  (SENSOR.TIMESTAMP);
        DEBUG_PRINT.print  (F(" :: in Byte: "));

        myArray[0] = (SENSOR.TIMESTAMP >> 24) & 0xFF;
        myArray[1] = (SENSOR.TIMESTAMP >> 16) & 0xFF;
        myArray[2] = (SENSOR.TIMESTAMP >>  8) & 0xFF;
        myArray[3] =  SENSOR.TIMESTAMP & 0xFF;

        for(int x = 0; x <= 3; x++){
          DEBUG_PRINT.print(" ");
          DEBUG_PRINT.print(myArray[x]);
        }
        
        Wire.write(myArray, 4);
        DEBUG_PRINT.println(" ");
      break;


      default :
        DEBUG_PRINT.println(F(":: >> EMPTY <<"));
        Wire.write("ERR_02");
        break;
    }
  } // end if( reg >= 0)
  else{
    DEBUG_PRINT.print(":: ERR Wrong Register:");
    DEBUG_PRINT.println(reg);
    Wire.write("ERR_01");
  }

    reg = -1;     // if trransmistion go well then set regster for unreal value to prewent resend the same information 
    DEBUG_PRINT.println("");
}

TwoWire rtcc = TwoWire(0);

void INIT_I2C(){
  DEBUG_PRINT.println("INIT I2C:");
  DEBUG_PRINT.print("I2C address: ");  
  DEBUG_PRINT.print((I2C_ADDRESS));

  Wire.begin(I2C_ADDRESS, 7, 6,100000); //I2C_ADDRESS, I2C_SDA, I2C_SCL, speed);      /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
}



void IRAM_ATTR isr() {
  ESP.restart();
        // digitalWrite( A1, !digitalRead(A1));

}

void setup() {
  randomSeed(analogRead(0));
  pinMode( A1 , OUTPUT);
  digitalWrite( A1, LOW);

	pinMode(A0, INPUT_PULLUP);
	attachInterrupt(A0, isr, FALLING);

  for(int i=0;i<=10;i++){
      digitalWrite( A1, !digitalRead(A1));
      delay(100);
  }



  DEBUG_PRINT.begin(9600);
  DEBUG_PRINT.println(" ");
  DEBUG_PRINT.println("======== SETUP ======== ");
  INIT();
  INIT_I2C();
  
}
  byte a =0;
  uint32_t last = 0;

void loop() {
  int dt =0;
  dt = (millis() - last);
  DEBUG_PRINT.print("ms: ");        DEBUG_PRINT.print(dt);
  DEBUG_PRINT.print(" \t :: Alarm : ");  DEBUG_PRINT.print(bitRead(SENSOR.ALARM,0)); 
  DEBUG_PRINT.print(" [");  DEBUG_PRINT.print((uint8_t)(SENSOR.ALARM >> 1)); DEBUG_PRINT.print("]");
 
  DEBUG_PRINT.print(" \t :: Status: ");  DEBUG_PRINT.print(bitRead(SENSOR.STATUS,0));
  DEBUG_PRINT.print(" [");  DEBUG_PRINT.print((uint8_t)(SENSOR.STATUS >> 1)); DEBUG_PRINT.print("]");

  DEBUG_PRINT.print(" \t :: ERROR: ");  DEBUG_PRINT.print(bitRead(SENSOR.ERROR,0));
  DEBUG_PRINT.print(" [");  DEBUG_PRINT.print((uint8_t)(SENSOR.ERROR >> 1)); DEBUG_PRINT.print("]");

  DEBUG_PRINT.print(" \t :: TIMESTAMP: ");  DEBUG_PRINT.print(SENSOR.TIMESTAMP);
  DEBUG_PRINT.print(" \t :: DET VALUE: ");  DEBUG_PRINT.print(SENSOR.DETECTION_VALUE);
  
  // e


  DEBUG_PRINT.print(" \t :: RESET: ");  DEBUG_PRINT.println(SENSOR.STATUS);



  if ( dt > 20000 ) {
    last = millis();
    INIT_RANDOM(); 
    DATAPROCESS();
  }
  SENSOR.TIMESTAMP = millis();
  SENSOR.DETECTION_VALUE++;
  
  DATAPROCESS();
  delay(500);
  
  // digitalWrite( A1, !digitalRead(A1));


}
