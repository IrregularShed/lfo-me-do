#include <Arduino.h>
#include <U8x8lib.h>
#include <SPI.h>
#include <AD9833.h>

// Pins that we need to keep track of
#define FSYNC 10
#define MODE 4
#define SHAPE 3
#define RANGE 2
#define KNOB A0
#define CV A1

AD9833 ad9833(FSYNC);
WaveformType wf = SINE_WAVE;
uint8_t source = KNOB;

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

uint8_t sineTile[8] = {12, 2, 2, 12, 48, 64, 64, 48};
uint8_t squareTile[8] = {64, 126, 2, 2, 2, 126, 64, 64};
uint8_t triangleTile[8] = {96, 24, 6, 24, 96, 24, 6, 24};

// filtering for potentiometer
float EMA_a = 0.5;
int EMA_S = 0;

// so we don't spam the module with updates
int currentVal = 0;

void drawWaveform(uint8_t waveForm[])
{
  u8x8.drawTile(13, 4, 1, waveForm);
  u8x8.drawTile(14, 4, 1, waveForm);
  u8x8.drawTile(15, 4, 1, waveForm);
}

// Cycle through the waveform options
void changeWaveform()
{
  if (wf == SINE_WAVE)
  {
    wf = TRIANGLE_WAVE;
    drawWaveform(triangleTile);
  }
  else if (wf == TRIANGLE_WAVE)
  {
    wf = SQUARE_WAVE;
    drawWaveform(squareTile);
  }
  else
  {
    wf = SINE_WAVE;
    drawWaveform(sineTile);
  }

  delay(50);
  while (digitalRead(SHAPE) == LOW)
  {
  }
}

void changeSource()
{
  if (source == KNOB)
  {
    source = CV;
  }
  else
  {
    source = KNOB;
  }
  delay(50);
  while (digitalRead(MODE) == LOW)
  {
  }
}

void setup()
{
  ad9833.Begin(); // apparently HAS to be the first command

  u8x8.begin();
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  //u8x8.drawString(0, 1, "1.03");
  u8x8.setCursor(0, 1);
  u8x8.print(3.14);

  u8x8.setFont(u8x8_font_pressstart2p_r);
  u8x8.drawString(13, 2, "Hz");
  drawWaveform(sineTile);
  EMA_S = analogRead(source);

  ad9833.ApplySignal(wf, REG0, 440);
  ad9833.EnableOutput(true);

  pinMode(MODE, INPUT_PULLUP);
  pinMode(SHAPE, INPUT_PULLUP);
  pinMode(RANGE, INPUT_PULLUP);
}

void loop()
{
  int potVal = analogRead(source);
  EMA_S = (EMA_a * potVal) + ((1 - EMA_a) * EMA_S);
  byte mappedVal = map(potVal, 0, 1021, 100, 1); // because I wired the pot up the wrong way round :|

  if (mappedVal != currentVal)
  {
    currentVal = mappedVal;
    // this should be a float
    ad9833.ApplySignal(wf, REG0, currentVal);

    u8x8.setCursor(0, 1);
    u8x8.setFont(u8x8_font_inb33_3x6_n);

    if (mappedVal < 10)
    {
      u8x8.print("   ");
    }
    else if (mappedVal < 100)
    {
      u8x8.print("  ");
    }
    else if (mappedVal < 1000)
    {
      u8x8.print(" ");
    }

    u8x8.print(mappedVal);
  }

  u8x8.setFont(u8x8_font_pressstart2p_r);
  char buttons[] = "   ";

  if (digitalRead(MODE) == LOW)
  {
    buttons[0] = 'M';
    changeSource();
  }

  if (digitalRead(SHAPE) == LOW)
  {
    buttons[1] = 'S';
    changeWaveform();
    ad9833.ApplySignal(wf, REG0, currentVal);
  }

  if (digitalRead(RANGE) == LOW)
  {
    buttons[2] = 'R';
  }

  u8x8.drawString(13, 0, buttons);

  delay(5);
}