#include "driver/i2s.h"
#include <WiFi.h>
#include "base.c"

/************** 增益调节+led显示 *************/
#include <ShiftRegister74HC595.h>
#define SDI 21
#define SCLK 4
#define LOAD 2
#define SDI2 26
#define SCLK2 25
#define LOAD2 33

// create shift register object (number of shift registers, data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr (SDI, SCLK, LOAD);
ShiftRegister74HC595<2> sr2 (SDI2, SCLK2, LOAD2);


int value, digit1, digit2, digit3, digit4;
uint8_t  digits[] = {B11000000, //0
                     B11111001, //1
                     B10100100, //2
                     B10110000, //3
                     B10011001, //4
                     B10010010, //5
                     B10000010, //6
                     B11111000, //7
                     B10000000, //8
                     B10010000 //9
                    };
/**
   显示增益
*/
void showNumber(int num)
{
  digit2 = num % 10 ;
  digit1 = num / 10 ;
  //Send them to 7 segment displays
  uint8_t numberToPrint[] = {digits[digit1], digits[digit2]};
  sr.setAll(numberToPrint);
}
/**
   显示亮度
*/
void showNumber2(int num)
{
  digit2 = num % 10 ;
  digit1 = num / 10 ;
  //Send them to 7 segment displays
  uint8_t numberToPrint[] = {digits[digit1], digits[digit2]};
  sr2.setAll(numberToPrint);
}

byte rotaryPinGa = 18;
byte rotaryPinGb = 19;
byte rotaryPinGa2 = 12;
byte rotaryPinGb2 = 13;
// 增益,增益指的是对频率最大采样值的增益，增益变大，频谱的参考值会变高
float gain = 50;

#include "RotateCounter.h"


void gainCallback(int a,int b){
  gain -= a;
  gain += b;
  showNumber(int(gain));
}

RotateCounter rc_gain(rotaryPinGa,rotaryPinGb);
void gainRotateInit(){
  showNumber(gain);
  
  rc_gain.check(gainCallback);
  attachInterrupt(
    digitalPinToInterrupt(rc_gain.rotaryPinGa),
    gainInterruptHandler,
    CHANGE);
  attachInterrupt(
    digitalPinToInterrupt(rc_gain.rotaryPinGb),
    gainInterruptHandler, 
    CHANGE);
}
void gainInterruptHandler(){
  rc_gain.isrGx();
}

/***********  时间 **************/
#include "time.h"
//#include "base.c";
//#include "led.h"
#include "esp_timer.h"
bool maoDis = false;

uint8_t baseX = 2;
uint8_t baseY = 1;

uint32_t lastTime = esp_timer_get_time() / 1000;
uint8_t colorIndexStart = 0;
uint8_t colorSize = 32;
const int NUMBER_LIGHTS[10][25] = {
  {1, 2, 3, 4, 6, 7, 9, 10, 12, 13, 14, 15},    //0
  {2, 5, 8, 11, 14},                            //1
  {1, 2, 3, 6, 9, 8, 7, 10, 13, 14, 15},        //2
  {1, 2, 3, 6, 7, 8, 9, 12, 13, 14, 15},        //3
  {1, 3, 4, 6, 7, 8, 9, 12, 15},      //4
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
      setXYColorCode(x, y, 0xEA4CD4);
    }

  }
}
void displayMaoAtXY(uint8_t x, uint8_t y) {
  setXYColorCode(x, y + 1, 0xEA4CD4);
  setXYColorCode(x, y + 3, 0xEA4CD4);


}

void displayBorder() {
  uint8_t borderSize = (kMatrixWidth - 1) * 2 + (kMatrixHeight - 1 - 2) * 2;
  uint8_t borderIndexs[borderSize] = {};


  //宽高各缩小一格
  for (uint8_t i = 0; i < kMatrixWidth - 1; i++) {
    borderIndexs[i] = i;
  }
  for (uint8_t i = 0; i < kMatrixHeight - 2; i++) {
    // x = kMatrixWidth-2; y = (i+1);
    borderIndexs[kMatrixWidth - 1 + i] = (i + 1) * kMatrixWidth + kMatrixWidth - 2;
  }
  for (uint8_t i = 0; i < kMatrixWidth - 1; i++) {
    // x = kMatrixWidth - 2 - i,y = kMatrixHeight-2
    borderIndexs[kMatrixWidth - 1 + kMatrixHeight - 3 + i] = (kMatrixHeight - 2) * kMatrixWidth + kMatrixWidth - 2 - i;
  }
  for (uint8_t i = 0; i < kMatrixHeight - 2; i++) {
    // x = 0, y  = kMatrixHeight -3-i
    borderIndexs[kMatrixWidth - 1 + kMatrixHeight - 3 + kMatrixWidth - 1 + i] = (kMatrixHeight - 3 - i) * kMatrixWidth;
  }
  colorIndexStart++;
  if (colorIndexStart >= colorSize) {
    colorIndexStart = 0;
  }
  for (uint8_t i = 0; i < borderSize; i++) {
    uint8_t x = borderIndexs[i] % kMatrixWidth;
    uint8_t y = borderIndexs[i] / kMatrixWidth;
    uint8_t colorIndexNow = colorIndexStart + i;

    while (colorIndexNow >= colorSize) {
      colorIndexNow = colorIndexNow - colorSize;
    }
    setXYColorFlow(x, y, colorIndexNow);


  }
  //


}

void displayMao() {
  uint32_t nowTime = esp_timer_get_time() / 1000;
  if (nowTime - lastTime >= 500) {
    maoDis = !maoDis;

    lastTime = nowTime;
  }
  if (maoDis) {
    displayMaoAtXY(baseX + 4 + 4, baseY);
    displayMaoAtXY(baseX + 4 + 4 + 1 + 1 + 4 + 4, baseY);
  }

}
void display(uint8_t hour, uint8_t minite, uint8_t second)
{
  setAllBlack();
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
/***************************************/

#include <FastLED.h>
//#include "base.c";
//#include "led.h";

// 每一列的led色值
#define LED_PIN 23
#define BRIGHTNESS 25
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define MAX_DIMENSION ((kMatrixWidth > kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
CRGB leds[256] = {};
uint32_t LED_COLORS[32] = {
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
  Serial.println(sizeof(leds));
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

void setXYColorCode(uint8_t x, uint8_t y, uint32_t code) {
  leds[XY(x, y)].setColorCode(code);
}
void setXYColorFlow(uint8_t x, uint8_t y, uint8_t index) {

  leds[XY(x, y)].setColorCode(LED_COLORS[index]);
}

void setAllBlack() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void ledShow() {
  LEDS.show();
}






float startIndexs[kMatrixWidth]  ;
float maxValues[kMatrixWidth]  ;
int displayMode = 1; // 显示模式，1 频谱，2 时钟
int displayModeDelay = 30;


#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
  These values can be changed in order to evaluate the functions
*/
#define CHANNEL A0
const i2s_port_t I2S_PORT = I2S_NUM_0;
const uint16_t BLOCK_SIZE = 512; //This value MUST ALWAYS be a power of 2
const uint16_t BUFFER_SIZE = 8;
const double samplingFrequency = 44100; //Hz, must be less than 10000 due to ADC

double vReal[BLOCK_SIZE];
double vImag[BLOCK_SIZE];
int32_t samples[BLOCK_SIZE];

const char* ssid       = "十一";
const char* password   = "keaide_11";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * 8;
const int   daylightOffset_sec = 0;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  display(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void setupMic()
{
  Serial.println("Configuring I2S...");
  esp_err_t err;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
    .sample_rate = 16000,                              // 16KHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,      // could only get it to work with 32bits
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,      // although the SEL config should be left, it seems to transmit on right
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
    .dma_buf_count = 8,                       // number of buffers
    .dma_buf_len = BLOCK_SIZE                 // samples per buffer
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
    .bck_io_num = 14,   // BCKL
    .ws_io_num = 15,    // LRCL
    .data_out_num = -1, // not used (only for speakers)
    .data_in_num = 32   // DOUT
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK)
  {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true)
      ;
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK)
  {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true)
      ;
  }
  Serial.println("I2S driver installed.");
}
const int i2s_num = 0;
int retStat = 0;
int32_t sampleIn = 0;

void setup()
{
  initLed();
  setAllBlack();
  ledShow();
  //connect to WiFi
  Serial.begin(115200);
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);




  for (uint8_t i = 0; i < sizeof(startIndexs); ++i)
    startIndexs[i] = 7;
  for (uint8_t i = 0; i < sizeof(maxValues); ++i)
    maxValues[i] = 0;
  //
  //
  setupMic();
  //
  delay(1000);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  gainRotateInit();
}


void loop()
{

  int num_bytes_read = i2s_read_bytes(I2S_PORT,
                                      (char *)samples,
                                      BLOCK_SIZE,     // the doc says bytes, but its elements.
                                      portMAX_DELAY); // no timeout
  int samples_read = num_bytes_read / 8;
  for (uint16_t i = 0; i < BLOCK_SIZE; i++)
  {
    vReal[i] = samples[i] >> 14;
    vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  }
  //  for(int i=0;i<BLOCK_SIZE/8;i++){
  //    Serial.println(vReal[i]);
  //  }
  //
  FFT.Windowing(vReal, BLOCK_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, BLOCK_SIZE, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, BLOCK_SIZE);

  rc_gain.check(gainCallback);
  displaySound(vReal);
  checkMode();
  if (displayMode == 2) {
    printLocalTime();
  }
  ledShow();

  delay(5);
}
double avgs[kMatrixWidth];
double maxV = 50000;
double allMaxV = 0;
uint16_t validSize = (BLOCK_SIZE / 2); //有效长度减去2，去掉最头上的两个值
int startIndex = 50;
const uint16_t stepV = validSize / kMatrixWidth; // 跳步，最后会余下 validSize%ledLen，没什么用直接丢掉了
uint16_t stayLowTime = 0;
uint16_t stayHighTime = 0;
void displaySound(double *vData)
{
  
  setAllBlack();
  uint16_t i = 0;
  uint16_t j = 0;
  uint16_t firstLight;
  allMaxV = 0;
  for (i = 0; i < kMatrixWidth; i++)
  {
    //采样数64，取8个值，每个值都是8个值平均值。
    double maxValue = 0;

    for (j = 0; j < stepV; j++)
    {
      //      maxValue += vData[i * kMatrixWidth + j];
      double v = vData[i * stepV + j + startIndex];
      //      maxValue = max(maxValue, v);
      maxValue += v;
    }
    allMaxV = max(allMaxV, maxValue / stepV);
    avgs[i] = maxValue / stepV;
    //    Serial.print(i);
    //    Serial.print(":");
    //        Serial.println(maxValue);
    //    Serial.print(":");
    //    Serial.print(index);
    //    Serial.print(":");
    //    Serial.println(vData[index]);
  }
  for (i = 0; i < kMatrixWidth; i++)
  {
    firstLight = kMatrixHeight - 1;
    for (j = 0; j < kMatrixHeight; j++)
    {
      // 平均值 * 增益（默认10）/参考值
      if ((avgs[i] * gain / maxV) > (float(kMatrixHeight - j) / (float(kMatrixHeight))))
      {
        firstLight = j;
        break;
      }
    }

    if (startIndexs[i] >= firstLight)
    {
      startIndexs[i] = firstLight;
    }
    else
    {
      startIndexs[i] = startIndexs[i] + 0.5;
    }
    if (startIndexs[i] > 7)
    {
      // 金色不会小时
      startIndexs[i] = 7;
    }
    for (j = startIndexs[i]; j < kMatrixHeight; j++)
    {
      setXYColorFlow(i, j, i);
      //      leds[XY(i, j)].setColorCode(LED_COLORS[i]);
    }
    //    leds[XY(i, floor(startIndexs[i]))] = CRGB::Gold;
    //    leds[XY(i , kMatrixHeight - 1)] = CRGB::Blue;
  }

  return;
}

void checkMode () {
  if (allMaxV < 100) {
    stayLowTime += 1;
    stayHighTime = 0;
  } else {
    stayHighTime += 1;
    stayLowTime = 0;
  }
  if (stayLowTime > displayModeDelay) {
    displayMode = 2;
  }
  if (stayHighTime > displayModeDelay) {
    displayMode = 1;
  }

}
