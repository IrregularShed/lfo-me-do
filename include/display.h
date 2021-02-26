#ifndef __twat__
#define __twat__

#include <U8x8lib.h>

extern uint8_t sineTile[8];
extern uint8_t squareTile[8];
extern uint8_t triangleTile[8];

void drawWaveform(uint8_t waveForm[]);

void setupDisplay();

void displayFrequencyNumber(float freq);

void displayButtonStates(char buttons[]);

#endif