#include <Arduino.h>
#include <Wire.h>
#include "main.h"
#include <stdint.h>


#define DEBUG  true
#define DEBUG_PRINT if(DEBUG) Serial
#define N 10


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
  float_t  DETECTION_VALUE_BKG;
  
  uint8_t  STATUS;                          /* Dev. Status. bit 0:  0 - device off or error or 1 - Normal operating mode 
                                                bit 1-7  converted to uint8. Values from 0 - 128 statsu details slist will be provided later */
  uint8_t  ERROR;                           /* Dev. ERROR. bit 0:  0 - no error or 1 - some error 
                                                bit 1-7  converted to uint8. Values from 0 - 128 error details list will be provided later */
  uint8_t  ALARM;                           /* Dev. ALARM. bit 0:  0 - no alarm or 1 - alarm. device reach value higher than treshold 
                                                bit 1-7  converted to uint8. Values from 0 - 128 alarm details list will be provided later */
  uint8_t  RESET;                           /* Dev. RESET. bit 0-7: vonverted to uint8. indyvidual value mens reset of indyvidual component or full devices. 
                                                only for servcies, debiging, or is case of freez*/

  uint64_t RUUNINGTIME;

  uint32_t TIMESTAMP;

  uint32_t ID_CHIP;
  
  uint32_t FW;

  uint16_t IP1, IP2, IP3, IP4, IP5;
  bool I2C_run;

  bool RUN;
  uint32_t loop_load_avg;  // Indicative loop load average
  uint32_t uptime;                          // Counting every second until 4294967295 = 130 year
  
  bool    MODE_UPDATE;
  bool    WILLALARM;

  uint8_t MODE;       /*  0 - OFF
                          1 - AUTO
                          2 - MANUAL
                          3 - ERROR */
};


sensorData_t SENSOR;
byte reg;

void INIT_DEV_PARAM(){          // prepare init values 
  DEBUG_PRINT.println(":: INIT PARAM..."); 

  SENSOR.ID                     = DEVICE_1_ID; 
  SENSOR.ID_CHIP                = SENSOR_1_ID; 
  SENSOR.FW                     = 0x1603FF10  ;

  SENSOR.RESET                  = 0;
  SENSOR.STATUS                 = 125;
  SENSOR.ERROR                  = 0;
  SENSOR.IP1                    = 0;
  SENSOR.IP2                    = 0;
  SENSOR.IP3                    = 0;
  SENSOR.IP4                    = 0;
  SENSOR.IP5                    = 0;
  SENSOR.ALARM                  = 0;
  SENSOR.DETECTION_TRESHOLD     = 123;
  SENSOR.DETECTION_VALUE        = 0;
  SENSOR.DETECTION_VALUE_BKG    = 0;
  SENSOR.TIMESTAMP              = 0;
  SENSOR.MODE                   = 0;
  SENSOR.MODE_UPDATE            = false;
  SENSOR.RUUNINGTIME            = 0;
  SENSOR.I2C_run                = false;
  SENSOR.WILLALARM              = false;

}


void DATAPROCESS_RESET(){
  DEBUG_PRINT.print("I2C RESET DEVICE: ");
  if(SENSOR.RESET == 128)  DEBUG_PRINT.println (SENSOR.RESET); // ESP.restart();   // perform HardReset
  if(SENSOR.RESET == 129)  DEBUG_PRINT.println (SENSOR.RESET);
  if(SENSOR.RESET == 130)  DEBUG_PRINT.println (SENSOR.RESET);

}


void  DATAPROCESS(){

  if(SENSOR.RESET != 0)  DATAPROCESS_RESET();

  if(SENSOR.DETECTION_VALUE > SENSOR.DETECTION_TRESHOLD){
    uint8_t tmp = SENSOR.ALARM >> 1;
    tmp++;
    tmp<<1;
    bitWrite(SENSOR.ALARM,0,1);

  } else {
    bitWrite(SENSOR.ALARM,0,0);
  }


}

void softwareReset() {
  DEBUG_PRINT.println  (":: ERROR Software Restart");
  esp_restart();
  while(1) { DEBUG_PRINT.print  (".");}
}

/*===============================================*/
/*===============================================*/
void I2C_write_32bit( uint32_t data_to_send){
  DEBUG_PRINT.print  (F(" \t :: 4 bytes :: "));
  DEBUG_PRINT.print  (data_to_send);
  DEBUG_PRINT.print  (F(" :: in Byte: "));
  
  byte myArray[4];
  
  myArray[0] = ( data_to_send >> 24) & 0xFF;
  myArray[1] = ( data_to_send >> 16) & 0xFF;
  myArray[2] = ( data_to_send >>  8) & 0xFF;
  myArray[3] = ( data_to_send      ) & 0xFF;

  for(int x = 0; x <= 3; x++){
    DEBUG_PRINT.print(" ");
    DEBUG_PRINT.print(myArray[x]);
  }

  Wire.write(myArray, 4);
  DEBUG_PRINT.println(" ");
}

void I2C_write_16bit( uint16_t data_to_send){
  DEBUG_PRINT.print  (F(" \t :: 2 bytes :: "));
  DEBUG_PRINT.print  (data_to_send);
  DEBUG_PRINT.print  (F(" :: in Byte: "));

  byte myArray[2];

  myArray[0] = (data_to_send >>  8) & 0xFF;
  myArray[1] = (data_to_send      ) & 0xFF;

  for(int x = 0; x <= 1; x++){
    DEBUG_PRINT.print(" ");
    DEBUG_PRINT.print(myArray[x]);
  }

  Wire.write(myArray, 2);
  DEBUG_PRINT.println(" ");
}

void I2C_write_8bit( uint16_t data_to_send){
  DEBUG_PRINT.print  (F(" \t :: 1 bytes :: "));
  DEBUG_PRINT.print  (data_to_send);
  DEBUG_PRINT.print  (F(" :: in Byte: "));

  byte myArray[1];

  myArray[0] = (data_to_send      ) & 0xFF;

  for(int x = 0; x <= 0; x++){
    DEBUG_PRINT.print(" ");
    DEBUG_PRINT.print(myArray[x]);
  }

  Wire.write(myArray, 1);
  DEBUG_PRINT.println(" ");
}

uint8_t I2C_read_8bit( int howMany){
  byte read;
  if( howMany == 2 and Wire.available() ){
    read = 0;
    read = Wire.read();
  }
  else{     
    DEBUG_PRINT.println(":: ERR4");
    return 0;
  }
  DEBUG_PRINT.print(read);
  DEBUG_PRINT.print(" >> ");
  return read;
}

uint16_t I2C_read_16bit( int howMany){
  uint16_t read;
  if( howMany == 3 and Wire.available() ){

    read = 0;
    read = Wire.read();
    read = (read << 8) | Wire.read();

  }
  else{     
    DEBUG_PRINT.println(":: ERR4");
    return 0;
  }
  DEBUG_PRINT.print(read);
  DEBUG_PRINT.print(" >> ");
  return read;
}

uint32_t I2C_read_32bit( int howMany){
  uint32_t read;
  if( howMany == 5 and Wire.available() ){

    read = 0;
    read = Wire.read();
    while ( Wire.available() )  read = (read << 8) | Wire.read();

  }
  else{     
    DEBUG_PRINT.println(":: ERR4");
    return 0;
  }
  DEBUG_PRINT.print(read);
  DEBUG_PRINT.print(" >> ");
  return read;
}

/*===============================================*/
/*===============================================*/

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
        SENSOR.RESET = I2C_read_8bit(howMany);
        DEBUG_PRINT.println(SENSOR.RESET);
        break;

      /*========================
      Device INTERNAL PARAM 1: 0x05 [R/W] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_1:
        SENSOR.IP1 = I2C_read_16bit(howMany);
        DEBUG_PRINT.println(SENSOR.IP1);
        break;

      /*========================
      Device INTERNAL PARAM 2: 0x06 [R/W] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_2:
        SENSOR.IP2 = I2C_read_16bit(howMany);
        DEBUG_PRINT.println(SENSOR.IP2);
        break;

      /*========================
      Device INTERNAL PARAM 3: 0x07 [R/W] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_3:
        SENSOR.IP3 = I2C_read_16bit(howMany);
        DEBUG_PRINT.println(SENSOR.IP3);
        break;

      /*========================
      Device INTERNAL PARAM 4: 0x08 [R/W] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_4:
        SENSOR.IP4 = I2C_read_16bit(howMany);
        DEBUG_PRINT.println(SENSOR.IP4);
        break;

      /*========================
      Device INTERNAL PARAM 5: 0x09 [R/W] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_5:
        SENSOR.IP5 = I2C_read_16bit(howMany);
        DEBUG_PRINT.println(SENSOR.IP5);
        break;

     /*========================
      Device DETECTION_TRESHOLD: 0x0A [ R/W ] 
      unsigned 32 bit integer [uint32_t]
      Read: 4 byte of data
      ========================*/
      case REG_DETECTION_TRESHOLD:
        SENSOR.DETECTION_TRESHOLD = I2C_read_32bit(howMany);
        DEBUG_PRINT.println(SENSOR.DETECTION_TRESHOLD);
        break;

     /*========================
      REG_TIMESTAMP: 0x0F [R/W] 
      unsigned 32 bit integer [uint32_t]
      Read: 4 byte of data
      ========================*/
      case REG_TIMESTAMP:
        SENSOR.TIMESTAMP = I2C_read_32bit(howMany);
        DEBUG_PRINT.println(SENSOR.TIMESTAMP);
        break;


      default :
        DEBUG_PRINT.println(F(":: ERR [3] Wrong Register:"));
        Wire.write("ERR3");
        break;
    }
}

// function executes whenever data is received from master
void receiveEvent(int howMany) {
  digitalWrite(PIN_LED2_RED, !digitalRead(PIN_LED2_RED));

  if( Wire.available()){
    reg = Wire.read();
  }
  DEBUG_PRINT.print  (":: <<<< Receive: Register: "); 
  DEBUG_PRINT.print  (reg);
  DEBUG_PRINT.print  (" \t :: ");
  DEBUG_PRINT.print  (howMany);
  DEBUG_PRINT.println(" bytes");


  if (howMany > 1) SaveToRegister(reg,howMany); 
}


// function executes whenever data is requested from master
void requestEvent() {
  DEBUG_PRINT.print  (":: >>>> Send:    Register: "); 
  DEBUG_PRINT.print  (reg);

  byte myArray[4];
  byte myArray16[2];

  if (reg > 0){
    switch(reg){
      /*========================
      Device ID: 0x01 [READ ONLY] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_ID:
        I2C_write_32bit(SENSOR.ID);
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
        I2C_write_8bit(SENSOR.STATUS);
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
        I2C_write_16bit(SENSOR.ERROR);
        break;

      /*========================
      Device INTERNAL PARAM 1: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_1:
        I2C_write_16bit(SENSOR.IP1);
      break;

      /*========================
      Device INTERNAL PARAM 2: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_2:
        I2C_write_16bit(SENSOR.IP2);
      break;


      /*========================
      Device INTERNAL PARAM 3: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_3:
        I2C_write_16bit(SENSOR.IP3);
      break;


      /*========================
      Device INTERNAL PARAM 4: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_4:
        I2C_write_16bit(SENSOR.IP4);
      break;


      /*========================
      Device INTERNAL PARAM 5: 0x04 [READ ONLY] 
      unsigned 16 bit integer [uint32_t]
      SEND: 2 byte of data
      ========================*/
      case REG_IP_5:
        I2C_write_16bit(SENSOR.IP5);
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
        I2C_write_8bit(SENSOR.ALARM);
        digitalWrite(PIN_LED2_RED, !digitalRead(PIN_LED2_RED));

        break;


    /*========================
      Device DETECTION_TRESHOLD: 0x0A [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_DETECTION_TRESHOLD:
        I2C_write_32bit(SENSOR.DETECTION_TRESHOLD);
        break;


    /*========================
      Device DETECTION_VALUE: 0x0B [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_DETECTION_VALUE:
        I2C_write_32bit(SENSOR.DETECTION_VALUE);
        break;


      /*========================
      Device FW: 0x0D [ READ  ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_FW:
        I2C_write_32bit(SENSOR.FW);
        break;

      /*========================
      Device ID_CHIP: 0x0E [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_ID_CHIP:
        I2C_write_32bit(SENSOR.ID_CHIP);
        break;


      /*========================
      Device REG_TIMESTAMP: 0x0E [ READ / WRITE ] 
      unsigned 32 bit integer [uint32_t]
      SEND: 4 byte of data
      ========================*/
      case REG_TIMESTAMP:
        I2C_write_32bit(SENSOR.TIMESTAMP);
        break;

      /*======================
      Error Wrong Register.
      Register are not supported
      =======================*/
      default :
        DEBUG_PRINT.println(F(":: ERR [2] Wrong Register:"));
        Wire.write("ERR2");
        break;
    }
  } // end if( reg >= 0)
  else{
    DEBUG_PRINT.print(":: ERR [1] Empty Register");
    DEBUG_PRINT.println(reg);
    Wire.write("ERR1");
  }

    reg = -1;     // if trransmistion go well then set regster for unreal value to prewent resend the same information 
    DEBUG_PRINT.println("");
}

void INIT_I2C(){
  DEBUG_PRINT.print(":: INIT I2C:\n");
  DEBUG_PRINT.print("::    * I2C mode:    "); DEBUG_PRINT.println("SLAVE");
  DEBUG_PRINT.print("::    * I2C address: "); DEBUG_PRINT.println(I2C_ADDRESS);
  DEBUG_PRINT.print("::    * SDA PIN:     "); DEBUG_PRINT.println(PIN_I2C_SDA);
  DEBUG_PRINT.print("::    * SCL PIN:     "); DEBUG_PRINT.println(PIN_I2C_SCL);


  // Wire.begin(I2C_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL,100000) ;
  
  Wire.begin(I2C_ADDRESS, 7, 8,100000) ;

  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  // if( Wire.begin(I2C_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL,100000) == true ){
  //   Wire.onReceive(receiveEvent);
  //   Wire.onRequest(requestEvent);
  //   SENSOR.I2C_run = true;
  // }else{
  //   DEBUG_PRINT.println(":: INIT I2C ERROR ...... device will run in offline mode. Please use Serial communication via RS232.");
  //   SENSOR.I2C_run = false;
  // }
}



// void IRAM_ATTR isr() {
//   ESP.restart();
// }
void IRAM_ATTR isr() {
  detachInterrupt(7);
	SENSOR.WILLALARM = true;
  DEBUG_PRINT.print(">>>>>>>>>>>>>>>>>>>>>>>>>> WILL ALARM <<<<<<<<<<<<<<<<<<<<<<<<<<");
}


void INIT_LED(){
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED2_RED  , OUTPUT);
  pinMode(PIN_LED2_GREEN, OUTPUT);
  digitalWrite(PIN_LED2_RED,0);
  digitalWrite(PIN_LED2_GREEN,0);
  digitalWrite(PIN_LED_GREEN,0);
}

void INIT_PUMPS(){
  pinMode(PIN_PUMP_1  , OUTPUT);
  digitalWrite(PIN_PUMP_1, LOW);
  
  pinMode(PIN_PUMP_2  , OUTPUT);
  digitalWrite(PIN_PUMP_2, LOW);
}

void INIT_FAN(){
  pinMode(PIN_FAN_1,OUTPUT);
  pinMode(PIN_FAN_2,OUTPUT);
  // pinMode(PIN_FAN_EN,OUTPUT);
}


void setup() {
delay(5000);
  DEBUG_PRINT.begin(9600);
  DEBUG_PRINT.println(" ");
  DEBUG_PRINT.println("======== SETUP START ========");

  Serial.begin(9600);
  
  INIT_DEV_PARAM();
  INIT_LED();
  // INIT_PUMPS();
  // INIT_FAN();
  INIT_I2C();

  DEBUG_PRINT.println("======== SETUP END ========");

  // pinMode(7, INPUT_PULLUP);
	// attachInterrupt(7, isr, FALLING);

  SENSOR.TIMESTAMP=millis();
  digitalWrite(PIN_LED2_RED,1);
  digitalWrite(PIN_LED2_GREEN,1);
  digitalWrite(PIN_LED_GREEN,1);
  // SENSOR.MODE = 1;

}


void PUMP_1(bool run){

  digitalWrite(PIN_FAN, !run);
}


void FAN(bool run){
    digitalWrite(PIN_PUMP_1,!run);

}

bool PUMP_1(){
  return digitalRead(PIN_PUMP_1);
}

void PUMP_2(bool run){
    digitalWrite(PIN_PUMP_2,!run);

}

bool PUMP_2(){
  return digitalRead(PIN_PUMP_2);
}


uint16_t ii = 0;
uint16_t history[N];
uint32_t total = 0 ;

void add_sample (uint16_t sample)
{
  uint16_t previous = history[ii] ;  // get oldest from buffer
  history[ii] = sample ;  // insert newest
  ii ++ ;    // move pointer circularly
  if (ii >= N)
    ii = 0 ;

  total -= previous ;  // update the cached sum of N latest samples to reflect
  total += sample ;    // the outgoing and incoming values
}

float TDS_value()
{
  return float (total) / N ;  // current average
}

void TDS(){
    add_sample(analogReadMilliVolts(PIN_DTS));
    SENSOR.DETECTION_VALUE = (int32_t)TDS_value();
    // Serial.println(TDS_value());
}




  // byte a = 0;
uint32_t last = 0;
uint64_t Sensor_start_time=0;

void BioSensor_Update(){

  if(SENSOR.MODE==1){
    if (Sensor_start_time == 0) Sensor_start_time=SENSOR.TIMESTAMP;
    SENSOR.RUUNINGTIME=SENSOR.TIMESTAMP-Sensor_start_time;

    Serial.println(" ");
    Serial.print(SENSOR.TIMESTAMP);
    Serial.print(" \t ");
    Serial.print(SENSOR.RUUNINGTIME);
    Serial.print(" \t ");

    if(SENSOR.RUUNINGTIME > 1 && SENSOR.RUUNINGTIME <=11){
      Serial.print("[1] PUMP_1 ON  \t PUMP_2 ON \t TDS OFF \t FAN OFF"); 
      Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
      Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
      Serial.print(" \t A:   "); Serial.print(SENSOR.ALARM);
      Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);

      PUMP_1(true);
      PUMP_2(true);
      FAN(0);
    }

    if(SENSOR.RUUNINGTIME > 11 && SENSOR.RUUNINGTIME <21){
      Serial.print("[2] PUMP_1 ON  \t PUMP_2 OFF \t TDS ON \t FAN OFF"); 
      Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
      Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
      Serial.print(" \t A:   "); Serial.print(SENSOR.ALARM);
      Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);

      PUMP_1(true);
      PUMP_2(false);
      FAN(0);
      TDS();
    }

    if(SENSOR.RUUNINGTIME > 21 && SENSOR.RUUNINGTIME <40){
      if(SENSOR.DETECTION_VALUE_BKG < 0 ) SENSOR.DETECTION_VALUE_BKG = TDS_value();
      Serial.print("[3] PUMP_1 OFF \t PUMP_2 OFF \t TDS OFF \t FAN >ON<");     
      Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
      Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
      Serial.print(" \t A:   "); Serial.print(SENSOR.ALARM);
      Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);
      PUMP_1(false);
      PUMP_2(false);
      FAN(255);
      
    }

    if(SENSOR.RUUNINGTIME > 40 && SENSOR.RUUNINGTIME <=60){
      Serial.print("[4] PUMP_1 OFF \t PUMP_2 ON \t TDS ON \t FAN OFF");  
      Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
      Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
      Serial.print(" \t A:   "); Serial.print(SENSOR.ALARM);
      Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);

      PUMP_1(false);
      PUMP_2(true);
      FAN(0);
      TDS();
    }

    if(SENSOR.RUUNINGTIME > 60 && SENSOR.RUUNINGTIME > 0  ){
      PUMP_1(false);
      PUMP_2(false);
      FAN(0);
      SENSOR.RUUNINGTIME=0;
      SENSOR.MODE = 0;
      // Sensor_start_time=0;
      Serial.println("\n--------------- END ----------------\n");
      // if(SENSOR.WILLALARM) SENSOR.ALARM=2;
    }
  }
}




uint32_t uptime;
uint8_t Proc=0;
uint64_t ProcTime=0;


void LED_UPDATE(){
  int tryb=0;
  if(SENSOR.MODE==0)  tryb=0;
  if(SENSOR.MODE==1)  tryb=1;
  if(SENSOR.ALARM==1) tryb=2;

  switch(tryb){
      case 0:
        digitalWrite(PIN_LED2_GREEN, LOW);
        digitalWrite(PIN_LED2_RED  , LOW);
        break;
      case 1:
        if(SENSOR.TIMESTAMP%2)digitalWrite(PIN_LED_GREEN, !digitalRead(PIN_LED_GREEN));
        digitalWrite(PIN_LED2_RED  , LOW);
        break;
      case 2:
        digitalWrite(PIN_LED2_GREEN  , LOW);
        digitalWrite(PIN_LED2_RED    , HIGH);
        break;
      default :
        digitalWrite(PIN_LED2_GREEN, LOW);
        digitalWrite(PIN_LED2_RED  , LOW);
        break;
    }
}


void splitCommand(String text) {
  String val[3];
  int command_words=0;
  int start=0;

  for (int i = 0; i < text.length(); i++) {
    if (text.substring(i, i+1) == " " ) {
      val[command_words++] = text.substring(start, i);
      start = i;
    }
    val[command_words] = text.substring(start, text.length());
  }



}

bool checkStringIsNumerical(String myString)
{
  uint16_t Numbers = 0;

  for(uint16_t i = 0; i < myString.length(); i++)
  {
     if (myString[i] == '0' || myString[i] == '1' || myString[i] == '2' || myString[i] == '3' || myString[i] == '4' || myString[i] == '5' || 
         myString[i] == '6' || myString[i] == '7' || myString[i] == '8' || myString[i] == '9')
    {
        Numbers ++;
    }
  }

  if (Numbers == myString.length()) return true;
  else return false;
}

void SERIAL_COMMAND_GET(){
  int command_words =0;
  String val[5];
  int start=0;

  if(Serial.available() != 0) {             //wait for data available
    String teststr = Serial.readString();   //read until timeout
    teststr.trim();                         // remove any \r \n whitespace at the end of the String
    
    char char_array[teststr.length()+1];
    teststr.toCharArray(char_array,teststr.length()+1);

    char * pch;
    pch = strtok(char_array ," ");
    while(pch != NULL){
      val[command_words++] = String(pch);
      pch =  strtok(NULL, " ");
    }

    if(val[0] == "start"){
      DEBUG_PRINT.print(":: SENSOR START >>>>>");
      SENSOR.MODE_UPDATE= true;
      return;
      }

    if(val[0] == "id_chip" && val[1]==NULL && val[2]==NULL){
      DEBUG_PRINT.print(":: ID_CHIP:");
      DEBUG_PRINT.println(SENSOR.ID_CHIP);
      return;
    }

    if(val[0] == "id_chip"  && checkStringIsNumerical(val[1])){
      
      DEBUG_PRINT.print(":: ID_CHIP: ");
      DEBUG_PRINT.print(SENSOR.ID_CHIP);
      DEBUG_PRINT.print(" :: >> ");
      DEBUG_PRINT.println(val[1].toInt());
      SENSOR.ID_CHIP = val[1].toInt() ;
      return;
    }

    if(val[0] == "timestamp" && val[1]==NULL && val[2]==NULL){
      DEBUG_PRINT.print(":: TIMESAMP:");
      DEBUG_PRINT.println(SENSOR.TIMESTAMP);
      return;
    }

    if(val[0] == "alarm" && val[1]==NULL && val[2]==NULL){
      DEBUG_PRINT.print(":: ALARM:");
      DEBUG_PRINT.println(SENSOR.ALARM);
      SENSOR.ALARM=2;
      return;
    }

    if(val[0] == "timestamp"  && checkStringIsNumerical(val[1])){
      
      DEBUG_PRINT.print(":: TIMESTAMP: ");
      DEBUG_PRINT.print(SENSOR.TIMESTAMP);
      DEBUG_PRINT.print(" :: >> ");
      DEBUG_PRINT.println(val[1].toInt());
      SENSOR.TIMESTAMP = val[1].toInt() ;
      return;
    }

    DEBUG_PRINT.print(":: Wrong command:\n");

  }
}




uint32_t timestamp_old=0;


void XdrvXsnsCall(uint32_t function) {
  // XdrvCall(function);
  // XsnsCall(function);
}


inline int32_t TimeDifference(uint32_t prev, uint32_t next)
{
  return ((int32_t) (next - prev));
}

int32_t TimePassedSince(uint32_t timestamp)
{
  // Compute the number of milliSeconds passed since timestamp given.
  // Note: value can be negative if the timestamp has not yet been reached.
  return TimeDifference(timestamp, millis());
}

bool TimeReached(uint32_t timer)
{
  // Check if a certain timeout has been reached.
  const long passed = TimePassedSince(timer);
  return (passed >= 0);
}

void SleepDelay(uint32_t mseconds) {
  if ( mseconds ) {
    uint32_t wait = millis() + mseconds;
    while (!TimeReached(wait) && !Serial.available()) {  // We need to service serial buffer ASAP as otherwise we get uart buffer overrun
      delay(1);
    }
  } else {
    delay(0);
  }
}





void loop() {
  if (millis() % 50 == 0)  {
    SENSOR.TIMESTAMP=millis()/100;
    digitalWrite(PIN_LED2_GREEN,!digitalRead(PIN_LED2_GREEN));
    }

    FAN(0);
    delay(10000);
    FAN(1);
    delay(10000);


    // if (Sensor_start_time == 0) Sensor_start_time=millis()/1000;
    // SENSOR.RUUNINGTIME=(millis()/1000)-Sensor_start_time;

    // Serial.println(" ");
    // Serial.print(SENSOR.TIMESTAMP);
    // Serial.print(" \t ");
    // Serial.print(SENSOR.RUUNINGTIME);
    // Serial.print(" \t ");

    // if(SENSOR.RUUNINGTIME > 1 && SENSOR.RUUNINGTIME <=11){
    //   Serial.print("[1] PUMP_1 ON  \t PUMP_2 ON \t TDS OFF \t FAN OFF"); 
    //   Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
    //   Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
    //   Serial.print(" \t A:   "); Serial.print(SENSOR.ALARM);
    //   Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);

    //   PUMP_1(true);
    //   PUMP_2(true);
    //   FAN(0);
    // }

    // if(SENSOR.RUUNINGTIME > 11 && SENSOR.RUUNINGTIME <21){
    //   Serial.print("[2] PUMP_1 ON  \t PUMP_2 OFF \t TDS ON \t FAN OFF"); 
    //   Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
    //   Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
    //   Serial.print(" \t A:   "); Serial.print(SENSOR.ALARM);
    //   Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);

    //   PUMP_1(true);
    //   PUMP_2(false);
    //   FAN(0);
    //   TDS();
    // }

    // if(SENSOR.RUUNINGTIME > 21 && SENSOR.RUUNINGTIME <40){
    //   if(SENSOR.DETECTION_VALUE_BKG < 0 ) SENSOR.DETECTION_VALUE_BKG = TDS_value();
    //   Serial.print("[3] PUMP_1 OFF \t PUMP_2 OFF \t TDS OFF \t FAN >ON<");     
    //   Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
    //   Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
    //   Serial.print(" \t A:   "); Serial.print(SENSOR.ALARM);
    //   Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);
    //   PUMP_1(false);
    //   PUMP_2(false);
    //   FAN(255);
      
    // }

    // if(SENSOR.RUUNINGTIME > 40 && SENSOR.RUUNINGTIME <=60){
    //   Serial.print("[4] PUMP_1 OFF \t PUMP_2 ON \t TDS ON \t FAN OFF");  
    //   Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
    //   Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
    //   Serial.print(" \t A:   "); Serial.print(SENSOR.ALARM);
    //   Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);
    //   if(SENSOR.WILLALARM) SENSOR.ALARM=1;
    //   PUMP_1(false);
    //   PUMP_2(true);
    //   FAN(0);
    //   TDS();
    // }

    // if(SENSOR.RUUNINGTIME > 60 && SENSOR.RUUNINGTIME > 0  ){
    //   PUMP_1(false);
    //   PUMP_2(false);
    //   FAN(0);
    //   SENSOR.RUUNINGTIME=0;
    //   SENSOR.MODE = 0;
    //   Sensor_start_time=0;
    //   Serial.println("\n--------------- END ----------------\n");
    //   // if(SENSOR.WILLALARM) SENSOR.ALARM=2;
    // }

  

  


  // uint32_t timestamp_old=0;
  // uint32_t loop_delay = 500;
  // SENSOR.uptime = (uint32_t)(millis()/1000);
  // SENSOR.TIMESTAMP += (uint32_t)(millis()/1000) - timestamp_old;
  // timestamp_old = SENSOR.TIMESTAMP;


  // uint32_t my_sleep = millis();
  // delay(300);
  // uint32_t my_activity = millis() - my_sleep;

  // if( my_activity <(uint32_t)loop_delay) SleepDelay(loop_delay-my_activity);


  // if (!my_activity) { my_activity++; }             // We cannot divide by 0
  // if (!loop_delay) { loop_delay++; }               // We cannot divide by 0
  // uint32_t loops_per_second = 1000 / loop_delay;   // We need to keep track of this many loops per second
  // uint32_t this_cycle_ratio = 100 * my_activity / loop_delay;

  // SENSOR.loop_load_avg = SENSOR.loop_load_avg - (SENSOR.loop_load_avg / loops_per_second) + (this_cycle_ratio / loops_per_second); // Take away one loop average away and add the new one

  // Serial.print(" loop_load_avg >>>>>");
  // Serial.println(SENSOR.loop_load_avg);

  // Serial.print(" TIMESTAMP    >>>>>");
  // Serial.println(SENSOR.TIMESTAMP);
  // Serial.println((uint32_t)(millis()/1000));
  
  SERIAL_COMMAND_GET();


  // if(SENSOR.MODE_UPDATE){
  //   SENSOR.MODE_UPDATE=false;
  //   switch ((SENSOR.MODE))
  //   {
  //   case 0:
  //     Serial.println(":: >>>>> MODE:: 0");
  //     break;
  //   case 1:
  //     Serial.println(":: >>>>> MODE:: 1");
  //     break;
  //   case 2:
  //     Serial.println(":: >>>>> MODE:: 2");
  //     break;
  //   case 3:
  //     Serial.println(":: >>>>> MODE:: 3");
  //     break;
  //   case 4:
  //     Serial.println(":: >>>>> MODE:: 4");
  //     break;
    
    
  //   default:
  //     break;
  //   }
  // }

// BioSensor_Update();


/*  0 - OFF
1 - AUTO
2 - MANUAL
3 - ERROR */

  // switch (mmode)
  // {
  // case 0:
  //   Serial.println("N/A");
  //     Serial.print("[4] PUMP_1 OFF \t PUMP_2 ON \t TDS ON \t FAN OFF");  
  //     Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
  //     Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
  //     Serial.print(" \t A: "); Serial.print(SENSOR.ALARM);
  //     Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);
  //   break;
  // case 1:
  //   Serial.println("START");      
  //     Serial.print("[4] PUMP_1 OFF \t PUMP_2 ON \t TDS ON \t FAN OFF");  
  //     Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
  //     Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
  //     Serial.print(" \t A: "); Serial.print(SENSOR.ALARM);
  //     Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);
  //   SENSOR.MODE = 1;
  //   Sensor_start_time = 0;
  //   SENSOR.RUUNINGTIME = 0;
  //   break;
  // case 2:
  //   Serial.println("CLEAN");
  //     Serial.print("[4] PUMP_1 OFF \t PUMP_2 ON \t TDS ON \t FAN OFF");  
  //     Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
  //     Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
  //     Serial.print(" \t A: "); Serial.print(SENSOR.ALARM);
  //     Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);

  //   PUMP_1(HIGH);
  //   PUMP_2(HIGH);
  //   delay(3000);
  //   PUMP_2(LOW);
  //   delay(5000);
  //   PUMP_1(LOW);
  //   FAN(HIGH);
  //   delay(5000);
  //   FAN(LOW);
  //   PUMP_2(HIGH);
  //   delay(10000);
  //   PUMP_1(LOW);
  //   PUMP_2(LOW);
    
  //   break;

  // case 3:
  //   Serial.println("empty");
  //     Serial.print("[4] PUMP_1 OFF \t PUMP_2 ON \t TDS ON \t FAN OFF");  
  //     Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
  //     Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
  //     Serial.print(" \t A: "); Serial.print(SENSOR.ALARM);
  //     Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);

  //   PUMP_2(HIGH);
  //   delay(10000);
  //   PUMP_2(LOW);

  //   break;
  //   case 4:
  //   Serial.println("LOAD");
  //     Serial.print("PUMP_1 OFF \t PUMP_2 ON \t TDS ON \t FAN OFF");  
  //     Serial.print(" \t BKG: "); Serial.print(SENSOR.DETECTION_VALUE_BKG); 
  //     Serial.print(" \t VAL: "); Serial.print(SENSOR.DETECTION_VALUE);
  //     Serial.print(" \t A: "); Serial.print(SENSOR.ALARM);
  //     Serial.print(" \t Tsh: "); Serial.print(SENSOR.DETECTION_TRESHOLD);

  //   PUMP_1(HIGH);
  //   PUMP_2(HIGH);
  //   delay(3000);
  //   PUMP_2(LOW);
  //   delay(3000);
  //   PUMP_1(LOW);
  //   delay(1000);
  //   PUMP_1(LOW);
  //   PUMP_2(LOW);
    
  //   break;

  // default:
  //   break;
  // }
  // }


  // BioSensor_Update();
  // DATAPROCESS();
  // LED_UPDATE();
  




  delay(250);
  
  


}
