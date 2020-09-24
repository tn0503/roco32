#include <Wire.h>

#define PCA9685_ADDRESS 0x40
#define MODE_1 0x0
#define LED0_ON_L 0x06
#define PRE_SCALE 0xFE

#define PCA9685_OE 19
#define SDA 21
#define SCL 22

// action
#define STOP  0
#define ACT1  1
#define ACT2  2

int16_t tempAngles[12] = {90,90,90,90,90,90,90,90,90,90,90,90};
int16_t stopAngles[12] = {90,90,90,90,90,90,90,90,90,90,90,90};
int16_t act1Angles[4][13] = {
  { 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 8},
  { 95, 90, 90, 90, 90, 90, 85, 90, 90, 90, 90, 90, 8},
  {100, 90, 90, 90, 90, 90, 80, 90, 90, 90, 90, 90, 8},
  { 95, 90, 90, 90, 90, 90, 85, 90, 90, 90, 90, 90, 8}
};
int16_t act2Angles[4][13] = {
  { 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 8},
  { 95, 95, 90, 90, 90, 90, 85, 85, 90, 90, 90, 90, 8},
  {100,100, 90, 90, 90, 90, 80, 80, 90, 90, 90, 90, 8},
  { 95, 95, 90, 90, 90, 90, 85, 85, 90, 90, 90, 90, 8}
};
int motionAngles[4][13];
uint8_t maxRows;
uint8_t divCounter;
uint8_t keyFrame;
uint8_t nextKeyFrame;
uint8_t actionMode;

// remote
const uint8_t interruptPin = 26;
boolean  rmReceived = 0;  //信号受信完了した
uint8_t  i;               //受信データの桁
uint8_t  rmState = 0;     //信号受信状況
uint8_t  dataCode;        //データコード(8bit)
uint8_t  rmData;        //データコード(8bit)外部用
uint8_t  invDataCode;     //反転データコード(8bit)
uint16_t customCode;      //カスタムコード(16bit)
uint32_t rmCode;          //コード全体(32bit)
volatile uint32_t prevMicros = 0; //時間計測用

void rmUpdate() //信号が変化した
{
  uint32_t width; //パルスの幅を計測
  if(rmState != 0){
    width = micros() - prevMicros;  //時間間隔を計算
    if(width > 10000)rmState = 0; //長すぎ
    prevMicros = micros();  //次の計算用
  }
  switch(rmState){
    case 0: //信号未達
    prevMicros = micros();  //現在時刻(microseconds)を記憶
    rmState = 1;  //最初のOFF->ON信号を検出した
    i = 0;
    return;
    case 1: //最初のON状態
      if((width > 9500) || (width < 8500)){ //リーダーコード(9ms)ではない
        rmState = 0;
      }else{
        rmState = 2;  //ON->OFFで9ms検出
      }
      break;
    case 2: //9ms検出した
      if((width > 5000) || (width < 4000)){ //リーダーコード(4.5ms)ではない
        rmState = 0;
      }else{
        rmState = 3;  //OFF->ONで4.5ms検出
      }
      break;
    case 3: //4.5ms検出した
      if((width > 700) || (width < 400)){
        rmState = 0;
      }else{
        rmState = 4;  //ON->OFFで0.56ms検出した
      }
      break;
    case 4: //0.56ms検出した
      if((width > 1800) || (width < 400)){  //OFF期間(2.25-0.56)msより長い or (1.125-0.56)msより短い
        rmState = 0;
      }else{
        if(width > 1000){ //OFF期間長い -> 1
          bitSet(rmCode, (i));
        }else{             //OFF期間短い -> 0
          bitClear(rmCode, (i));
        }
        i++;  //次のbit
        if(i > 31){ //完了
          rmReceived = 1;
          return;
        }
        rmState = 3;  //次のON->OFFを待つ
      }
      break;
    }
}

void set_angle() {
  int angle;
  Wire.beginTransmission(0x40);
  Wire.write(LED0_ON_L);
  for(int i=0;i<12;i++){
    angle = 400 + (tempAngles[i] - 90)*2.8;
    Wire.write(0x0);
    Wire.write(0x0>>8);
    Wire.write(angle);
    Wire.write(angle>>8);
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
  
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), rmUpdate, CHANGE);

  divCounter = 0;
  keyFrame = 0;
  nextKeyFrame = 1;
}

void loop() {
  if(rmReceived){ //リモコン受信した
    detachInterrupt(digitalPinToInterrupt(interruptPin));
    rmState = 0;      //初期化
    //図とは左右が逆であることに注意
    customCode = rmCode;    //下16bitがcustomCode
    dataCode = rmCode >> 16;  //下16bitを捨てたあとの下8bitがdataCode
    invDataCode = rmCode >> 24; //下24bitを捨てたあとの下8bitがinvDataCode
    if((dataCode + invDataCode) == 0xff){   //反転確認
      if (rmData == 12) { //1 button
        actionMode = ACT1;
        memcpy(motionAngles, act1Angles, sizeof(act1Angles));
        maxRows = sizeof(act1Angles) / sizeof(*act1Angles) - 1;
      } else if (rmData == 24) { //2 button
        actionMode = ACT2;
        memcpy(motionAngles, act2Angles, sizeof(act2Angles));
        maxRows = sizeof(act2Angles) / sizeof(*act2Angles) - 1;
      } else if (rmData == 94) { //3 button
        actionMode = STOP;
        for(int i=0; i<12; i++) {
          tempAngles[i] = stopAngles[i];
        }
        set_angle();
      }  
    }
    rmReceived = 0;
    attachInterrupt(digitalPinToInterrupt(interruptPin), rmUpdate, CHANGE);
  }
  if(actionMode != STOP){
    divCounter++;
    if(divCounter >= motionAngles[nextKeyFrame][12]) {
      divCounter = 0;
      keyFrame = nextKeyFrame;
      nextKeyFrame++;
      if(nextKeyFrame > maxRows) nextKeyFrame = 0;
    }
    for(int i=0; i<12; i++) {
      tempAngles[i] = motionAngles[keyFrame][i] + 
        int8_t((motionAngles[nextKeyFrame][i] - motionAngles[keyFrame][i])
        * divCounter / motionAngles[nextKeyFrame][12]);
    }
    set_angle();
    delay(30);
  }
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
