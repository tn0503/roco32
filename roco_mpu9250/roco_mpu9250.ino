#include <Wire.h>

#define MPU9250_ADDRESS 0x68
#define PWR_MGMT_1 0x6B

#define GYRO_CONFIG 0x1B
#define GYRO_FS_SEL_250 0x00
#define GYRO_FS_SEL_500 0x01
#define GYRO_FS_SEL_1000 0x02
#define GYRO_FS_SEL_2000 0x03

#define ACCEL_CONFIG 0x1C
#define ACCEL_FS_SEL_2 0x00
#define ACCEL_FS_SEL_4 0x01
#define ACCEL_FS_SEL_8 0x02
#define ACCEL_FS_SEL_16 0x03

#define AK8963_ADDRESS 0x0C
#define INT_PIN_CFG 0x37
#define CNTL1 0x0A
#define POWER_DOWN_MODE 0x00
#define SINGLE_MEAS_MODE 0x01
#define CONT_MEAS_MODE1 0x02 //8Hz
#define CONT_MEAS_MODE2 0x06 //100Hz
#define EXT_MEAS_MODE 0x04
#define SELF_TEST_MODE 0x08
#define FUSE_ROM_MODE 0x0F
#define BIT14_OUTPUT 0x00
#define BIT16_OUTPUT 0x10
#define ST1 0x02

volatile float acFullRange;
volatile float gyFullRange;
volatile uint8_t mpuData[14];
volatile uint8_t magData[7];
volatile int16_t aX, aY,aZ;
volatile int16_t gX,gY,gZ;
volatile int16_t mX,mY,mZ;
volatile float acX,acY,acZ;
volatile float gyX,gyY,gyZ;
volatile float mgX,mgY,mgZ;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  init_mpu9250();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  read_mpu9250();
  delay(200);
  Serial.print("ax :");
  Serial.print(acX);
  Serial.print(" ay :");
  Serial.print(acY);
  Serial.print(" az :");
  Serial.print(acZ);
  Serial.print(" gx :");
  Serial.print(gyX);
  Serial.print(" gy :");
  Serial.print(gyY);
  Serial.print(" gz :");
  Serial.print(gyZ);
  Serial.print(" mx :");
  Serial.print(mgX);
  Serial.print(" my :");
  Serial.print(mgY);
  Serial.print(" mz :");
  Serial.print(mgZ);
  Serial.println();
}

void read_mpu9250(){
  i2c_read(MPU9250_ADDRESS, 0x3B, 14, mpuData);
  uint8_t drdy = 0;
  i2c_read(AK8963_ADDRESS, ST1, 1, &drdy);
  if((drdy & 0x01)){
    i2c_read(AK8963_ADDRESS, 0x03, 7, magData);
  }
  aX = (mpuData[0] << 8) | mpuData[1];
  aY = (mpuData[2] << 8) | mpuData[3];
  aZ = (mpuData[4] << 8) | mpuData[5];
  gX = (mpuData[8] << 8) | mpuData[9];
  gY = (mpuData[10] << 8) | mpuData[11];
  gZ = (mpuData[12] << 8) | mpuData[13];
  //Orientation of axes are different. > datasheet p38
  mX = (magData[1] << 8) | magData[0];
  mY = (magData[3] << 8) | magData[2];
  mZ = (magData[5] << 8) | magData[4];

  acX = aX * acFullRange / 32768.0f;
  acY = aY * acFullRange / 32768.0f;
  acZ = aZ * acFullRange / 32768.0f;
  gyX = gX * gyFullRange / 32768.0f;
  gyY = gY * gyFullRange / 32768.0f;
  gyZ = gZ * gyFullRange / 32768.0f;
  mgX = mX / 32768.0f * 4800.0f;
  mgY = mY / 32768.0f * 4800.0f;
  mgZ = mZ / 32768.0f * 4800.0f;
}

void init_mpu9250(){
  i2c_write(MPU9250_ADDRESS, PWR_MGMT_1, 0x00);
  i2c_write(MPU9250_ADDRESS, GYRO_CONFIG, (GYRO_FS_SEL_2000)<<3);
  gyFullRange = 2000.0;
  i2c_write(MPU9250_ADDRESS, ACCEL_CONFIG, (ACCEL_FS_SEL_16)<<3);
  acFullRange = 16.0;
  i2c_write(MPU9250_ADDRESS, INT_PIN_CFG, 0x02);
  i2c_write(AK8963_ADDRESS, CNTL1, (BIT16_OUTPUT | CONT_MEAS_MODE2));
}

void i2c_write(uint8_t address, uint8_t location, volatile uint8_t dat){
  Wire.beginTransmission(address);
  Wire.write(location);
  Wire.write(dat);
  Wire.endTransmission();
}

void i2c_read(uint8_t address, uint8_t location, uint8_t howmany, volatile uint8_t* buff){
  Wire.beginTransmission(address);
  Wire.write(location);
  Wire.endTransmission();
  
  Wire.requestFrom(address, howmany);
  uint8_t i=0;
  while(Wire.available()){
    buff[i++] = Wire.read();
  }
}
