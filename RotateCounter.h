#ifndef ROTATE_COUNTER_H
#define ROTATE_COUNTER_H

#include <Arduino.h>



typedef void (*rotatecallback)(int, int);

class RotateCounter {
  private:
    bool rotaryInputDetected = false;
    byte rotationCountL = 0;
    byte rotationCountR = 0;
    bool rotaryBtnPushed = false;
    byte rotaryPushCount = 0;

    byte isrGxDebounceDuration = 2; // msec

    unsigned long isrGxTime = 0;
    unsigned long isrGxLastTime = 0;
    
    
    void init();
    enum rotationInputStatus {
      kRotationStatusWaiting, // Ga: HIGH and Gb: HIGH
      kRotationStatusStartLorPush, // Ga: FALLING, Gb: HIGH
      kRotationStatusStartR, // Ga: HIGH, Gb: FALLING
      kRotationStatusTurningL, // Ga: RISING, Gb: LOW
      kRotationStatusTurningR, // Ga: LOW, Gb: RISING
      kRotationStatusWaitForFinishL, // Ga: RISING, Gb: LOW
      kRotationStatusWaitForFinishR, // Ga: LOW, Gb RISING
    } rotationStatus;
  public:
    byte rotaryPinGa;
    byte rotaryPinGb;
    void isrGx();
    void check(rotatecallback p);
    /**
       初始化，分别传入两个pin
    */
    RotateCounter (byte GA, byte GB);
};

#endif
