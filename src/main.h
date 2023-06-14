
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




#define I2C_ADDRESS       0x66
#define I2C_SCL           13              // A5  - those pins can not be changed for arduino uno
#define I2C_SDA           12              // A4  - those pins can not be changed for arduino uno
#define PIN_ALARM         23
#define BIOSENSOR_ID      UINT32_MAX      // 24 bits unsigned integer. range: 0 - 16`777`215 
