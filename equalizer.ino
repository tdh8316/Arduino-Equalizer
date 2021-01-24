#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <arduinoFFT.h>


// 핀
#define PIN_DISPLAY 8
#define PIN_MAX9814 A0

// LED Matrix 밝기
#define BRIGHTNESS 8

// LED Matrix 크기
const int displayX = 16;
const int displayY = 16;

// 오디오 샘플링 매개변수
const uint16_t samples = displayX * 2;
const uint16_t bufferSize = samples >> 1;
// const unsigned long samplingFrequency = 16000;
const unsigned long samplingFrequency = 40000;

// 푸리에 변환 클래스 선언
arduinoFFT fft = arduinoFFT();

// 푸리에 변환할 주파수 샘플
double vReal[samples];
double vImag[samples];

// LED Matrix 객체 생성
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(displayX, displayY, PIN_DISPLAY,
                            NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                            NEO_RGB            + NEO_KHZ800);

// LED Matrix 색상
uint32_t RED = matrix.Color(255, 0, 0);
uint32_t GREEN = matrix.Color(0, 255, 0);
uint32_t BLUE = matrix.Color(0, 0, 255);

float x = 0;
int r = 255 * abs(sin(x * (180 / PI)));
int g = 255 * abs(sin((x + PI / 3) * (180 / PI)));
int b = 255 * abs(sin((x + (2 * PI) / 3) * (180 / PI)));

// (0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500)Hz 진폭
double amplitudes[displayX];

// 각 LED 열의 높이
uint8_t matrixArray[displayX];


void setup() {
  Serial.begin(115200);

  // LED Matrix 초기화
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(8);
  matrix.fillScreen(0);

  // 인터럽트 중지
  cli();
}

void loop() {
  x = x + 0.0001;
  
  // 음파 샘플링
  for (uint8_t i = 0; i < samples; i++) {
    vReal[i] = analogRead(PIN_MAX9814); // 음파의 진폭을 실수부에 할당
    vImag[i] = 0; // 허수부 초기화
  }

  // 전류에 의한 노이즈 제거
  fft.DCRemoval(vReal, samples);

  // 신호분석을 위한 Window Function
  fft.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);

  // 푸리에 변환
  fft.Compute(vReal, vImag, samples, FFT_FORWARD);

  // 복소평면상의 벡터의 크기 계산
  fft.ComplexToMagnitude(vReal, vImag, samples);

  for (uint16_t i = 0; i < bufferSize; i++) {
    // 주파수
    // double abscissa = ((i * 1.0 * samplingFrequency) / samples);

    // 진폭 범위 조정
    amplitudes[i] = ceil(vReal[i]);

    // 범위 조정
    matrixArray[i] = constrain(sq(log(amplitudes[i]) / log(3)) / 2, 0, displayY);
  }


  r = 255 * abs(sin(x * (180 / PI)));
  g = 255 * abs(sin((x + PI / 3) * (180 / PI)));
  b = 255 * abs(sin((x + (2 * PI) / 3) * (180 / PI)));

  Serial.print(r); Serial.print(","); Serial.print(g); Serial.print(","); Serial.println(b);

  // LED Matrix에 그리기
  matrix.fillScreen(0);
  for (uint16_t i = 0; i < bufferSize; i++) {
    uint8_t h = matrixArray[i];

    for (uint8_t j = 0; j < h; j++) {
      uint8_t point;
      if ((i + 1) % 2 == 1) {
        point = ((i + 1) * displayY) - 1 - j;
      } else {
        point = ((i + 1) * displayY) - displayY + j;
      }
      matrix.setPixelColor(point, matrix.Color(r, g, b));
    }
  }
  matrix.show();
  // PrintVector(vReal, bufferSize);
}


void PrintVector(double *vData, uint16_t bufferSize) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    double abscissa = ((i * 1.0 * samplingFrequency) / samples);

    Serial.print(abscissa, 0);
    Serial.print("Hz");
    Serial.print(":");
    Serial.print(vData[i], 2);
    Serial.print(",");
  }
  Serial.println();
}

void printAmps() {
  for (uint16_t i = 0; i < bufferSize; i++) {
    Serial.print(matrixArray[i]);
    Serial.print(",");
  }
  Serial.println();
}
