
#include <FastLED.h>

#include "led.h";

// 每一列的led色值
#define LED_PIN 23
#define BRIGHTNESS 75
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define MAX_DIMENSION ((kMatrixWidth > kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
 CRGB leds[NUM_LEDS];
 int LED_COLORS[32] = {
    0x4AF7A2,
    0x57FDD8,
    0x5CEEF9,
    0x43BCFA,
    0x4992F8,
    0x4567EE,
    0x4C44F1,
    0x854EFB,
    0xC151FB,
    0xDE47F1,
    0xEA4CD4,
    0xE33998,
    0xF53876,
    0xFE4A59,
    0xF7694B,
    0xFA9A52,
    0xFDC54E,
    0xFDC650,
    0xFF9D53,
    0xFE6E4E,
    0xFF4855,
    0xFA3E4E,
    0xF42E70,
    0xF644DA,
    0xF54DDF,
    0xBE50FF,
    0x884CFA,
    0x3D3BED,
    0x2D53F0,
    0x4A62F9,
    0x498DF5,
    0x47BAF8
    };
void initLed() {
  LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("led ready");
  Serial.println(NUM_LEDS);
  Serial.println(sizeof(leds),1);
}

uint16_t XY(uint8_t x, uint8_t y)
{
  // 每个 x 的起点 index
  int xStart = ((x + 1) / 2) * 16 + (x + 1) % 2;
  int realIndex = 0;
  if ((x + 1) % 2 == 0)
  {
    realIndex = xStart - y;
  }
  else
  {
    realIndex = xStart + y;
  }
  return realIndex - 1;
}

void setXYColorCode(uint8_t x, uint8_t y,uint32_t code){
  leds[XY(x, y)].setColorCode(code);
}
void setXYColorFlow(uint8_t x, uint8_t y,uint8_t index){

  leds[XY(x, y)].setColorCode(LED_COLORS[index]);
}

void setAllBlack(){
   fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void ledShow(){
  LEDS.show();
}
