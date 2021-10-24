#ifndef _LED_CPP
#define _LED_CPP
#include  <stdint.h>



// The leds
//extern CRGB leds[kMatrixWidth * kMatrixHeight];
//extern uint32_t LED_COLORS[32];
const uint8_t kMatrixWidth = 32;
const uint8_t kMatrixHeight = 8;
uint16_t XY(uint8_t x, uint8_t y);
void initLed();
void setAllBlack();
void setXYColorCode(uint8_t x, uint8_t y,uint32_t code);
void setXYColorFlow(uint8_t x, uint8_t y,uint8_t index);
void ledShow();
#endif
