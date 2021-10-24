
#include "base.h"
#include "led.h"
#include "clock.h"
#include <WiFi.h>

#include "driver/i2s.h"







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
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);
  Serial.begin(115200);

  for (uint8_t i = 0; i < sizeof(startIndexs); ++i)
    startIndexs[i] = 7;
  for (uint8_t i = 0; i < sizeof(maxValues); ++i)
    maxValues[i] = 0;
  pinMode(LED_PIN, OUTPUT);

  setupMic();
  //This pulls in a bunch of samples and does nothing, its just used to settle the mics output
  // for (retStat = 0; retStat < BLOCK_SIZE * 2; retStat++)
  // {
  //   i2s_pop_sample((i2s_port_t)i2s_num, (char *)&sampleIn, portMAX_DELAY);
  //   delay(1);
  // }
  delay(1000);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
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

  FFT.Windowing(vReal, BLOCK_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, BLOCK_SIZE, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, BLOCK_SIZE);

  
  displaySound(vReal);
  if(displayMode == 2){
    printLocalTime();
  }
  LEDS.show();

  delay(5);
}
double avgs[kMatrixWidth];
double maxV = 500;
double allMaxV = 0;
uint16_t validSize = (BLOCK_SIZE / 2); //有效长度减去2，去掉最头上的两个值
int startIndex = 50;
const uint16_t stepV = validSize / kMatrixWidth; // 跳步，最后会余下 validSize%ledLen，没什么用直接丢掉了
uint16_t stayLowTime = 0;
uint16_t stayHighTime = 0;
void displaySound(double *vData)
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);
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
      maxValue = max(maxValue, v);
    }
    allMaxV = max(allMaxV,maxValue);
    avgs[i] = maxValue;
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
      if (avgs[i] / maxV > (float(kMatrixHeight - j) / (float(kMatrixHeight))))
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
      leds[XY(i, j)].setColorCode(LED_COLORS[i]);
    }
    //    leds[XY(i, floor(startIndexs[i]))] = CRGB::Gold;
    //    leds[XY(i , kMatrixHeight - 1)] = CRGB::Blue;
  }
  
  return;
}

void checkMode (){
  if(allMaxV<100){
    stayLowTime+=1;
    stayHighTime=0;
  }else{
    stayHighTime+=1;
    stayLowTime = 0;
  }
  if(stayLowTime>displayModeDelay){
    displayMode = 2;
  }
  if(stayHighTime >displayModeDelay){
    displayMode = 1;
  }
  
}
