#ifndef __oledDisplay__
#define __oledDisplay__

#include <U8x8lib.h>

extern uint8_t sineTile[8];
extern uint8_t squareTile[8];
extern uint8_t triangleTile[8];

void drawWaveform(uint8_t waveForm[]);

void setupDisplay();

void displayFrequencyNumber(float freq, float precision);
void blankFrequency();
void displayCVmode(bool active);
void displayRange(char str[]);
#endif