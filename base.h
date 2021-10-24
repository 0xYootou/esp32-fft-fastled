const int kMatrixWidth = 32;
const int kMatrixHeight = 8;
const bool kMatrixSerpentineLayout = true;
const bool kMatrixVertical = true;

int displayMode = 1; // 显示模式，1 频谱，2 时钟
int displayModeDelay = 30;

float startIndexs[kMatrixWidth];
float maxValues[kMatrixWidth];
