#include <FastLED.h>
#include "driver/i2s.h"
#define LED_PIN     23
#define BRIGHTNESS  25
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
//  esptool.py --chip esp32 --port /dev/cu.wchusbserial53100038751 erase_flash
// esptool.py --chip esp32 --port /dev/cu.wchusbserial53100038751 --baud 460800 write_flash -z 0x1000 esp32-20190125-v1.10.bin
// screen /dev/cu.wchusbserial53100038751 -b115200
const uint8_t kMatrixWidth  = 32;
const uint8_t kMatrixHeight = 8;
const bool    kMatrixSerpentineLayout = true;

const bool    kMatrixVertical = true;


#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)

// The leds
CRGB leds[kMatrixWidth * kMatrixHeight];


#include "arduinoFFT.h"
float startIndexs[kMatrixWidth] ;
float maxValues[kMatrixWidth] ;
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
#define CHANNEL A0
const uint16_t BLOCK_SIZE = 1024; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 44100; //Hz, must be less than 10000 due to ADC

double vReal[BLOCK_SIZE];
double vImag[BLOCK_SIZE];
int32_t samples[BLOCK_SIZE];

i2s_config_t i2s_config = {
mode: (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
  sample_rate: 44100,
bits_per_sample: I2S_BITS_PER_SAMPLE_32BIT,
channel_format: I2S_CHANNEL_FMT_ONLY_LEFT,
communication_format: (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB),
intr_alloc_flags: ESP_INTR_FLAG_LEVEL1,
  dma_buf_count: 8,
dma_buf_len : BLOCK_SIZE
};

i2s_pin_config_t pin_config = {
  .bck_io_num = 32, //this is BCK pin
  .ws_io_num = 25, // this is LRCK pin
  .data_out_num = I2S_PIN_NO_CHANGE, // this is DATA output pin
  .data_in_num = 33   //DATA IN
};
const int i2s_num = 0;
int retStat = 0;
int32_t sampleIn = 0;


void setup() {
  delay(3000);
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);
  Serial.begin(115200);

   for(uint8_t i = 0; i < sizeof(startIndexs); ++i)
      startIndexs[i] = 7;
   for(uint8_t i = 0; i < sizeof(maxValues); ++i)
      maxValues[i] = 0;

//Set up pin 19 for data IN from the Mic to the esp32
  pinMode(33, INPUT);
  //Set up pin 21 and 25 as the BCK and LRCK pins
  pinMode(25, OUTPUT);
  pinMode(32, OUTPUT);
  //Init the i2s device
  i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin((i2s_port_t)i2s_num, &pin_config);
  i2s_start((i2s_port_t)i2s_num);
  //This pulls in a bunch of samples and does nothing, its just used to settle the mics output
  for (retStat = 0; retStat < BLOCK_SIZE * 2; retStat++)
  {
    i2s_pop_sample((i2s_port_t)i2s_num, (char*)&sampleIn, portMAX_DELAY);
    delay(1);
  }
}

void loop() {
  
  fill_solid(leds, NUM_LEDS, CRGB::Black);
   
sampleIn = 0;
  for (uint16_t i = 0; i < BLOCK_SIZE; i++)
  {
    //this reads 32bits as 4 chars into a 32bit INT variable
    i2s_pop_sample((i2s_port_t)i2s_num, (char*)&sampleIn, portMAX_DELAY);
    //this pushes out all the unwanted bits as we only need right channel data.
    sampleIn >>= 14;
    vReal[i] = sampleIn;
    vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  }
  FFT.Windowing(vReal, BLOCK_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, BLOCK_SIZE, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, BLOCK_SIZE);

  PrintVector(vReal, BLOCK_SIZE/2 );
  
  LEDS.show();
   
  delay(10);
  
}

void PrintVector(double *vData, uint16_t bufferSize )
{
  int index = 0;
  const uint8_t stepV = BLOCK_SIZE/2/kMatrixWidth;
 
  for(int i=0;i<kMatrixWidth;i++){
     //采样数64，取8个值，每个值都是8个值平均值。
     float total = 0;
     for( int j=0;j<stepV;j++){
        total+=vData[i*kMatrixWidth+j];
     }
    
     float avg = total/stepV;
     Serial.print(avg);
     Serial.print(',');
     Serial.print(total);
     Serial.print('|');
     Serial.print(stepV);
     Serial.print('&');
//     if(avg>maxValues[i]){
//       maxValues[i] = avg*1.2;
//     }
     int firstLight = kMatrixHeight-1;
     for(int t = 0;t<kMatrixHeight;t++){
      if(avg/1000>(float(kMatrixHeight-t)/(float(kMatrixHeight)))){
        
        firstLight = t;
        break;
      } 
     }
//     Serial.print(avg);
//     Serial.print(',');
      for(int n=firstLight+1;n<kMatrixHeight;n++){
        leds[XY(i,n)] = CRGB::Green;
      }
      
      float lastIndex = startIndexs[i];
      if(lastIndex >= firstLight){
        startIndexs[i] = firstLight;
      }else{
        startIndexs[i] = lastIndex+0.5;
      }
      leds[XY(i,floor(startIndexs[i]))] = CRGB::Gold;
      leds[XY(i,kMatrixHeight-1)] = CRGB::Blue;
  }
  Serial.println();
  return;
}



uint16_t XY( uint8_t x, uint8_t y)
{
  // 每个 x 的起点 index
  int xStart = ((x+1)/2)*16+(x+1)%2;
  int realIndex = 0;
  if((x+1)%2==0){
   realIndex = xStart-y;
  }else{
   realIndex = xStart+y;
  }
  return realIndex-1;
}

//
// Mark's xy coordinate mapping code.  See the XYMatrix for more information on it.
////
//uint16_t XY( uint8_t x, uint8_t y)
//{
//  uint16_t i;
//  
//  if( kMatrixSerpentineLayout == false) {
//    if (kMatrixVertical == false) {
//      i = (y * kMatrixWidth) + x;
//    } else {
//      i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
//    }
//  }
//
//  if( kMatrixSerpentineLayout == true) {
//    if (kMatrixVertical == false) {
//      if( y & 0x01) {
//        // Odd rows run backwards
//        uint8_t reverseX = (kMatrixWidth - 1) - x;
//        i = (y * kMatrixWidth) + reverseX;
//      } else {
//        // Even rows run forwards
//        i = (y * kMatrixWidth) + x;
//      }
//    } else { // vertical positioning
//      if ( x & 0x01) {
//        i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
//      } else {
//        i = kMatrixHeight * (kMatrixWidth - x) - (y+1);
//      }
//    }
//  }
//  
//  return i;
//}
