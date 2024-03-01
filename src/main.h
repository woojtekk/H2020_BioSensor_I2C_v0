
#define REG_ID                      0x01  // uint32 ==> 4 byte  R
#define REG_RESET                   0x02  //* uint8  ==> 1 byte  W
#define REG_STATUS                  0x03  // uint8  ==> 1 byte  R
#define REG_ERROR                   0x04  // uint8  ==> 1 byte  R
#define REG_IP_1                    0x05  //* uint16 ==> 2 byte  R/W
#define REG_IP_2                    0x06  //* uint16 ==> 2 byte  R/W
#define REG_IP_3                    0x07  //* uint16 ==> 2 byte  R/W
#define REG_IP_4                    0x08  //* uint16 ==> 2 byte  R/W
#define REG_IP_5                    0x09  //* uint16 ==> 2 byte  R/W
#define REG_ALARM                   0x0A  // uint8  ==> 1 byte  R
#define REG_DETECTION_TRESHOLD      0x0B  //* uint32 ==> 4 byte  R/W
#define REG_DETECTION_VALUE         0x0C  // uint32 ==> 4 byte  R
#define REG_FW                      0x0D  // uint32 ==> 4 byte  R
#define REG_ID_CHIP                 0x0E  // uint32 ==> 4 byte R
#define REG_TIMESTAMP               0x0F  // uint32 ==> 4 byte R/W

#define PIN_FAN_1       9 //
#define PIN_FAN_2       9 // 5

#define PIN_FAN         9 // 20   wentylator
#define PIN_PUMP_1      21 // 9   pompa lewa
#define PIN_PUMP_2      20 // 21  pompa prawa

#define PIN_DTS         1
#define PIN_TEMP        10

#define PIN_LED_GREEN   6 // 5 // 20
#define PIN_LED2_GREEN  4 // 4 // 21
#define PIN_LED2_RED    5 // 6 // 21

#define PIN_I2C_SDA    8 // 8 // 18
#define PIN_I2C_SCL    10 // 10 // 19


#define I2C_ADDRESS       0x66
// #define I2C_SCL           13              // A5  - those pins can not be changed for arduino uno
// #define I2C_SDA           12              // A4  - those pins can not be changed for arduino uno
#define PIN_ALARM         23

#define SENSOR_1_ID      0x1603FF09      // 24 bits unsigned integer. range: 0 - 16`777`215 
#define SENSOR_2_ID      0x1603FF09      // 24 bits unsigned integer. range: 0 - 16`777`215 
#define SENSOR_3_ID      0x1603FF09      // 24 bits unsigned integer. range: 0 - 16`777`215 

#define DEVICE_1_ID      231201      // 24 bits unsigned integer. range: 0 - 16`777`215 
#define DEVICE_2_ID      231202      // 24 bits unsigned integer. range: 0 - 16`777`215 
#define DEVICE_3_ID      231203      // 24 bits unsigned integer. range: 0 - 16`777`215 

