#include <display.h>

/*
   0123456789abcdef

 0 111222333444....
 1 111222333444.Hz.
 2 111222333444....
 3 111222333444.~~~
 4 111222333444....
 5 111222333444.>CV
 6 ................
 7 LFO 0 - 1 Hz.... or
 7 LFO 0 - 10 Hz... or
 7 Lin 0 - 100 Hz.. or
 7 Lin 0 - 1000 Hz. or
 7 1V/Octave, 5V... or
 7 1V/Octave, +/-1V or
 7 2 Octave range..
*/

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

uint8_t sineTile[8] = {12, 2, 2, 12, 48, 64, 64, 48};
uint8_t squareTile[8] = {64, 126, 2, 2, 2, 126, 64, 64};
uint8_t triangleTile[8] = {96, 24, 6, 24, 96, 24, 6, 24};

void drawWaveform(uint8_t waveForm[])
{
  u8x8.drawTile(13, 3, 1, waveForm);
  u8x8.drawTile(14, 3, 1, waveForm);
  u8x8.drawTile(15, 3, 1, waveForm);
}

void setupDisplay()
{
  u8x8.begin();
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  u8x8.setCursor(0, 0);
  u8x8.print("****");

  u8x8.setFont(u8x8_font_pressstart2p_r);
  u8x8.drawString(13, 1, "Hz");
  drawWaveform(sineTile);
}

void displayFrequencyNumber(float freq, float multiplier)
{
  int8_t precision;
  if (multiplier == 0.01)
  {
    precision = 2;
  }
  else if (multiplier == 0.1)
  {
    precision = 1;
  }
  else
  {
    precision = 0;
  }

  u8x8.setCursor(0, 0);
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  char charBuff[] = "    ";
  dtostrf(freq, 4, precision, charBuff);

  u8x8.print(charBuff);
}

void blankFrequency()
{
  //u8x8.setCursor(0, 0);
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  u8x8.drawString(0, 0, "----");
}

void displayCVmode(bool active)
{
  u8x8.setFont(u8x8_font_pressstart2p_r);
  u8x8.drawString(13, 5, active ? ">CV" : "   ");
}

void displayRange(char str[])
{
  u8x8.setFont(u8x8_font_pressstart2p_r);
  u8x8.drawString(0, 7, str);
}