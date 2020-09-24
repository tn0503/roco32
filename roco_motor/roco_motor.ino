const int motor1F = A12;
const int motor1R = A10;
const int motor2F = A14;
const int motor2R = A13;

void setup() {
  // put your setup code here, to run once:
  ledcSetup(0, 12800, 8);
  ledcAttachPin(motor1F, 0);
  ledcSetup(1, 12800, 8);
  ledcAttachPin(motor1R, 1);
  ledcSetup(2, 12800, 8);
  ledcAttachPin(motor2F, 2);
  ledcSetup(3, 12800, 8);
  ledcAttachPin(motor2R, 3);
  ledcWrite(0, 255);
  ledcWrite(1, 196);
  ledcWrite(2, 255);
  ledcWrite(3, 196);
}

void loop() {
  // put your main code here, to run repeatedly:

}
