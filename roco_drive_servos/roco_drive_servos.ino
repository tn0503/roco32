#include <Wire.h>

#define PCA9685_ADDRESS 0x40
#define MODE_1 0x0
#define LED0_ON_L 0x06
#define PRE_SCALE 0xFE

#define PCA9685_OE 19
#define SDA 21
#define SCL 22

int16_t tempAngles[12] = {90,90,90,90,90,90,90,90,90,90,90,90};

void set_angle() {
  int angle;
  
  Wire.beginTransmission(PCA9685_ADDRESS);
  Wire.write(LED0_ON_L);
  for(int i=0;i<12;i++){
    angle = 400 + (tempAngles[i] - 90)*2.8;
    Wire.write(0x0);       //led_on lower byte
    Wire.write(0x0>>8);    //led_on upper byte
    Wire.write(angle);     //led_off lower byte
    Wire.write(angle>>8);  //led_off upper byte
  }
  Wire.endTransmission();
}
 
void setup() {
  // put your setup code here, to run once:
  Wire.begin(SDA,SCL);
  Wire.setClock(100000);
 
  init_pca9685();
  pinMode(PCA9685_OE, OUTPUT);
  digitalWrite(PCA9685_OE,LOW);
  set_angle();
}
 
void loop() {
  // put your main code here, to run repeatedly:
 
}

void init_pca9685() {
  i2c_write(PCA9685_ADDRESS, MODE_1, 0x80);//reset device
  i2c_write(PCA9685_ADDRESS, MODE_1, 0x10);//sleep mode
  i2c_write(PCA9685_ADDRESS, PRE_SCALE, 0x65);//set prescaler to 60Hz
  i2c_write(PCA9685_ADDRESS, MODE_1, 0x80);//back to prev mode
  i2c_write(PCA9685_ADDRESS, MODE_1, 0xa0);//enable auto-increment
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
