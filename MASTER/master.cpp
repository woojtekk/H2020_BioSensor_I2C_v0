#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>


void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);  // start serial for output
  Wire.begin(0x10);        // join i2c bus (address optional for master)
}

void WriteToSlave(byte aa,int bb){
  uint32_t bigNum; 
  bigNum = random(1234567890);
  byte myArray[4];
  
  myArray[0] = (bigNum >> 24) & 0xFF;
  myArray[1] = (bigNum >> 16) & 0xFF;
  myArray[2] = (bigNum >> 8) & 0xFF;
  myArray[3] = bigNum & 0xFF;

  Serial.print("W REG::");
  Serial.print(aa);
  Serial.print(" \t [");
  Serial.print(bb);
  Serial.print("] --> ");

  Wire.beginTransmission(0x66);    // Get the slave's attention, tell it we're sending a command byte
  Wire.write(aa);   
  if(bb == 4 ) {
    Wire.write(myArray,4);
    Serial.println(bigNum);
    }
  else{
    Wire.write(127);
    Serial.println("127");
  } 

  Wire.endTransmission();
  delay(25);
}

void ReadFromSlave(byte aa,int bb){
  Wire.beginTransmission(0x66);    // Get the slave's attention, tell it we're sending a command byte
  Wire.write(aa);   
  Wire.endTransmission();
  delay(25);

  Wire.requestFrom(0x66, bb);

  Serial.print("R REG::");
  Serial.print(aa);
  Serial.print(" \t [");
  Serial.print(bb);
  Serial.print("] --> ");

  uint32_t bigNum;
  bigNum =0;
  memset(&bigNum,0,sizeof(bigNum));

  // byte a = 0;
  // byte b = 0;
  // byte c = 0;
  // byte d = 0;

  // a = Wire.read();
  // b = Wire.read();
  // c = Wire.read();
  // d = Wire.read();
  // bigNum = a;
  // bigNum = (bigNum << 8) | b;
  // bigNum = (bigNum << 8) | c;
  // bigNum = (bigNum << 8) | d;

  bigNum = Wire.read();

  while (Wire.available()) { 
      bigNum = (bigNum << 8) | Wire.read();
  }
  Serial.println(bigNum);
  // Serial.print("::");
  // Serial.print(a);
  // Serial.print("::");
  // Serial.print(b);
  // Serial.print("::");
  // Serial.print(c);
  // Serial.print("::");
  // Serial.println(d);


  // Wire.endTransmission();

}


void loop(){

  ReadFromSlave(0x01,4);
  delay(1000);

  ReadFromSlave(0x03,1);
  delay(1000);

  ReadFromSlave(0x04,1);
  delay(1000);

  ReadFromSlave(0x05,4);
  delay(1000);

  ReadFromSlave(0x06,4);
  delay(1000);

  ReadFromSlave(0x07,4);
  delay(1000);

  ReadFromSlave(0x08,4);
  delay(1000);

  ReadFromSlave(0x09,1);
  delay(1000);

  ReadFromSlave(0x0A,4);
  delay(1000);

  ReadFromSlave(0x0B,4);
  delay(1000);

  ReadFromSlave(0x0C,4);
  delay(1000);



  WriteToSlave(0x02,1);
  delay(1000);
  
  WriteToSlave(0x0A,4);
  delay(1000);  
  
  WriteToSlave(0x0B,4);
  delay(1000);
  
  WriteToSlave(0x0C,4);
  delay(1000);
  


}