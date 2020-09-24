#include "audiodata.h"

#define DAC_1 25

int wavCounter;

hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  if(wavCounter < sound_length) dacWrite(DAC_1, sound_data[wavCounter++]);
}

void setup() {
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 124, true);

  // Start an alarm
  timerAlarmEnable(timer);

  wavCounter = 0;
}

void loop() {
  
}
