#include "RotateCounter.h"

RotateCounter::RotateCounter(byte GA, byte GB) {
  rotaryPinGa = GA;
  rotaryPinGb = GB;
  
  init();
}


void RotateCounter::init(){
  rotationStatus = kRotationStatusWaiting;


  pinMode(rotaryPinGa, INPUT_PULLUP);
  pinMode(rotaryPinGb, INPUT_PULLUP);
  

}
void RotateCounter::check(rotatecallback p) {
  if (rotaryInputDetected) {

    (*p)(rotationCountL, rotationCountR);
    //    Serial.print("L: ");
    //    Serial.print(rotationCountL);
    //    Serial.print("; R: ");
    //    Serial.print(rotationCountR);
    //    Serial.print("; SW: ");
    //    Serial.print(percent);
    //    Serial.println();
    //    noInterrupts(); // necessary when reading a multi-byte variable from the ISR
    rotationCountL = 0;
    rotationCountR = 0;
    rotaryPushCount = 0;
    rotaryInputDetected = false;
    interrupts();
  }

}

void RotateCounter::isrGx() {
  /*
     This routine relies on not being interrupted by another (higher prioritzed) interrupt.
     This should be the case for the Arduino hardware.
  */


  byte rotationInputA = digitalRead(rotaryPinGa);
  byte rotationInputB = digitalRead(rotaryPinGb);

  isrGxTime = millis();

  switch (rotationStatus) {
    case kRotationStatusWaiting:
      if (HIGH == rotationInputA && LOW == rotationInputB) {
        rotationStatus = kRotationStatusStartR;
      }
      else if (LOW == rotationInputA && HIGH == rotationInputB) {
        rotationStatus = kRotationStatusStartLorPush;
        isrGxLastTime = isrGxTime; // debounce
      }
      break;

    case kRotationStatusStartLorPush:
      if (HIGH == rotationInputA && HIGH == rotationInputB) {
        rotationStatus = kRotationStatusWaiting;
        if ((isrGxTime - isrGxLastTime) > isrGxDebounceDuration) {
          ++rotaryPushCount;
          rotaryInputDetected = true;
        }
      }
      else if (LOW == rotationInputA && LOW == rotationInputB) {
        rotationStatus = kRotationStatusTurningL;
      }
      break;

    case kRotationStatusStartR:
      if (HIGH == rotationInputA && HIGH == rotationInputB) {
        rotationStatus = kRotationStatusWaiting;
      }
      else if (LOW == rotationInputA && LOW == rotationInputB) {
        rotationStatus = kRotationStatusTurningR;
      }
      break;

    case kRotationStatusTurningL:
      if (LOW == rotationInputA && HIGH == rotationInputB) {
        rotationStatus = kRotationStatusStartLorPush;
      }
      else if (HIGH == rotationInputA && LOW == rotationInputB) {
        rotationStatus = kRotationStatusWaitForFinishL;
      }
      break;

    case kRotationStatusTurningR:
      if (HIGH == rotationInputA && LOW == rotationInputB) {
        rotationStatus = kRotationStatusStartR;
      }
      else if (LOW == rotationInputA && HIGH == rotationInputB) {
        rotationStatus = kRotationStatusWaitForFinishR;
      }
      break;

    case kRotationStatusWaitForFinishL:
      if (LOW == rotationInputA && LOW == rotationInputB) {
        rotationStatus = kRotationStatusTurningL;
      }
      else if (HIGH == rotationInputA && HIGH == rotationInputB) {
        rotationStatus = kRotationStatusWaiting;
        rotationCountR = 0;
        ++rotationCountL;
        rotaryInputDetected = true;
        isrGxLastTime = isrGxTime; // debounce
      }
      break;

    case kRotationStatusWaitForFinishR:
      if (LOW == rotationInputA && LOW == rotationInputB) {
        rotationStatus = kRotationStatusTurningR;
      }
      else if (HIGH == rotationInputA && HIGH == rotationInputB) {
        rotationStatus = kRotationStatusWaiting;
        rotationCountL = 0;
        ++rotationCountR;
        rotaryInputDetected = true;
        isrGxLastTime = isrGxTime;
      }
      break;
  }
}
