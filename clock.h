#include "time.h"
const int NUMBER_LIGHTS[10][25] = {
  {1, 2, 3, 4, 6, 7, 9, 10, 12, 13, 14, 15},    //0
  {2, 5, 8, 11, 14},                            //1
  {1, 2, 3, 6, 9, 8, 7, 10, 13, 14, 15},        //2
  {1, 2, 3, 6, 7, 8, 9, 12, 13, 14, 15},        //3
  {1, 3,4,6,7,8,9,12,15},             //4
  {1, 2, 3, 4, 7, 8, 9, 12, 13, 14, 15},        //5
  {1, 2, 3, 4, 7, 8, 9, 10, 12, 13, 14, 15},    //6
  {1, 2, 3, 6, 9, 12, 15},                      //7
  {1, 2, 3, 4, 6, 7, 8, 9, 10, 12, 13, 14, 15}, //8
  {1, 2, 3, 4, 6, 7, 8, 9, 12, 15},             //9
  // {2, 3, 4, 7, 9, 12, 14, 17, 19, 22, 23, 24},
  // {3, 8, 13, 18, 23},
  // {2, 3, 4, 9, 12, 13, 14, 17, 22, 23, 24},
  // {2, 3, 4, 9, 12, 13, 14, 19, 22, 23, 24},
  // {2, 7, 12, 13, 14, 3, 8, 18, 23},
  // {2, 3, 4, 7, 12, 13, 14, 19, 24, 23, 22},
  // {2, 3, 4, 7, 12, 13, 14, 17, 19, 22, 23, 24},
  // {2, 3, 4, 9, 14, 19, 24},
  // {2, 3, 4, 7, 9, 12, 13, 14, 17, 19, 22, 23, 24},
  // {2, 3, 4, 7, 9, 12, 13, 14, 19, 24},
};

void displayNumberAtXY(uint8_t startX, uint8_t startY, uint8_t number)
{
  //  Serial.println();
  //  Serial.print(number);
  //  Serial.println();
  //计算其真实位置，并点亮
  for (uint8_t i = 0; i < 25; i++)
  {
    uint8_t lightIndex = NUMBER_LIGHTS[number][i] - 1; // 变成从0开始的值，范围 0~24
    if (lightIndex != 255) {
      uint8_t x_5x5 = lightIndex % 3;
      uint8_t y_5x5 = lightIndex / 3;
      uint8_t x = startX + x_5x5;
      uint8_t y = startY + y_5x5;
      //      Serial.print("lightIndex:");
      //      Serial.print(lightIndex);
      //      Serial.print(",x:");
      //      Serial.print(x);
      //      Serial.print(",y:");
      //      Serial.print(y);
      //      Serial.println();
      leds[XY(x, y)].setColorCode(0xEA4CD4);
    }

  }
}
bool maoDis = false;
void displayMaoAtXY(uint8_t x, uint8_t y) {

  leds[XY(x, y + 1)].setColorCode(0xEA4CD4);
  leds[XY(x, y + 3)].setColorCode(0xEA4CD4);

}
uint8_t baseX = 2;
uint8_t baseY = 1;
void display(uint8_t hour, uint8_t minite, uint8_t second)
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  uint8_t hour_1 = hour / 10;
  uint8_t hour_2 = hour %  10;

  uint8_t minite_1 = minite / 10;
  uint8_t minite_2 = minite % 10;

  uint8_t second_1 = second / 10;
  uint8_t second_2 = second % 10;
  
  displayNumberAtXY(baseX, baseY, hour_1);
  displayNumberAtXY(baseX + 4, baseY, hour_2);
 
  displayNumberAtXY(baseX + 4 + 4 + 1 + 1, baseY, minite_1);
  displayNumberAtXY(baseX + 4 + 4 + 1 + 1 + 4, baseY, minite_2);
  
  
  displayNumberAtXY(baseX + 4 + 4 + 1 + 1 + 4 + 4 + 1 + 1, baseY, second_1);
  displayNumberAtXY(baseX + 4 + 4 + 1 + 1 + 4 + 4 + 1 + 1 + 4, baseY, second_2);
  displayMao();
  displayBorder();
  
}
uint32_t lastTime = millis();
void displayMao(){
  uint32_t nowTime = millis();
  if(nowTime - lastTime >= 500){
    maoDis = !maoDis;
    
    lastTime = nowTime;
  }else{

    
  }
  if (maoDis) {
    displayMaoAtXY(baseX + 4 + 4, baseY);
    displayMaoAtXY(baseX + 4 + 4 + 1 + 1 + 4 + 4, baseY);
  }
  
}
uint8_t colorIndexStart = 0;
uint8_t colorSize = 32;
void displayBorder(){
  uint8_t borderSize = (kMatrixWidth-1)*2+(kMatrixHeight-1-2)*2;
  uint8_t borderIndexs[borderSize] = {};

  
  //宽高各缩小一格
  for(uint8_t i=0;i<kMatrixWidth-1;i++){
    borderIndexs[i] = i;
  }
  for(uint8_t i=0;i<kMatrixHeight-2;i++){
    // x = kMatrixWidth-2; y = (i+1);
    borderIndexs[kMatrixWidth-1+i] = (i+1)*kMatrixWidth + kMatrixWidth-2;
  }
  for(uint8_t i=0;i<kMatrixWidth-1;i++){
    // x = kMatrixWidth - 2 - i,y = kMatrixHeight-2
    borderIndexs[kMatrixWidth-1+kMatrixHeight-3+i] = (kMatrixHeight-2)*kMatrixWidth + kMatrixWidth - 2 - i;
  }
  for(uint8_t i=0;i<kMatrixHeight-2;i++){
    // x = 0, y  = kMatrixHeight -3-i
    borderIndexs[kMatrixWidth-1+kMatrixHeight-3+kMatrixWidth-1+i] = (kMatrixHeight -3-i)*kMatrixWidth;
  }
  colorIndexStart++;
  if(colorIndexStart>=colorSize){
    colorIndexStart = 0;
  }
  for(uint8_t i=0;i<borderSize;i++){
    uint8_t x = borderIndexs[i]%kMatrixWidth;
    uint8_t y = borderIndexs[i]/kMatrixWidth;
    uint8_t colorIndexNow = colorIndexStart+i;
    
    while(colorIndexNow>=colorSize){
      colorIndexNow = colorIndexNow - colorSize;
    }
    
    leds[XY(x, y)].setColorCode(LED_COLORS[colorIndexNow]);
    
  }
//  for(uint8_t i=0;i<kMatrixHeight;i++){
//    leds[XY(0, i)].setColorCode(0x4AF7A2);
//    leds[XY(kMatrixWidth-2, i)].setColorCode(0x4AF7A2);
//  }
  
  
}
