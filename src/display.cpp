#include <display.h>

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

uint8_t sineTile[8] = {12, 2, 2, 12, 48, 64, 64, 48};
uint8_t squareTile[8] = {64, 126, 2, 2, 2, 126, 64, 64};
uint8_t triangleTile[8] = {96, 24, 6, 24, 96, 24, 6, 24};

void drawWaveform(uint8_t waveForm[])
{
  u8x8.drawTile(13, 4, 1, waveForm);
  u8x8.drawTile(14, 4, 1, waveForm);
  u8x8.drawTile(15, 4, 1, waveForm);
}

void setupDisplay()
{
  u8x8.begin();
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  u8x8.setCursor(0, 1);
  u8x8.print(3.14);

  u8x8.setFont(u8x8_font_pressstart2p_r);
  u8x8.drawString(13, 2, "Hz");
  drawWaveform(sineTile);
}

void displayFrequencyNumber(float freq)
{
  u8x8.setCursor(0, 1);
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  char charBuff[] = "    ";

  dtostrf(freq, 4, 0, charBuff);

  u8x8.print(charBuff);
}

void displayButtonStates(char buttons[])
{
  u8x8.setFont(u8x8_font_pressstart2p_r);
  u8x8.drawString(13, 0, buttons);
}
