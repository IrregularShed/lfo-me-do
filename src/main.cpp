#include <Arduino.h>
#include <SPI.h>
#include <AD9833.h>
#include <display.h>
#include <SimpleKalmanFilter.h>
#include <math.h>

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
bool vPerOct = false;

// filtering for potentiometer
SimpleKalmanFilter filter(2, 2, 0.01);
//float EMA_a = 0.8;
//int EMA_S = 0;

// so we don't spam the module with updates
float currentVal = 0;

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

void changeRange()
{
  vPerOct = !vPerOct;
  delay(50);
  while (digitalRead(RANGE) == LOW)
  {
  }
}

float calculateFrequency(int potVal, int cvVal)
{
  // handle the filtering of inputs
  float filtered = filter.updateEstimate(potVal); // need to do CV soon

  if (vPerOct)
  {
    // do a volt-per-octave thing
    return (pow(2, filtered / 204.6) * 65.40639133);
  }
  else
  {
    // do a normal LFO thing
    return map(filtered, 0, 1023, 1, 100);
  }
}

void setup()
{
  ad9833.Begin(); // apparently HAS to be the first command

  setupDisplay();

  filter.updateEstimate(analogRead(source));

  ad9833.ApplySignal(wf, REG0, 440);
  ad9833.EnableOutput(true);

  pinMode(MODE, INPUT_PULLUP);
  pinMode(SHAPE, INPUT_PULLUP);
  pinMode(RANGE, INPUT_PULLUP);
}

void loop()
{
  // get input
  int potVal = 1023 - analogRead(KNOB); // because I wired the pot up the wrong way round :|
  int cvVal = analogRead(CV);
  bool modePressed = digitalRead(MODE) == LOW;
  bool shapePressed = digitalRead(SHAPE) == LOW;
  bool rangePressed = digitalRead(RANGE) == LOW;

  // process input
  float mappedVal = calculateFrequency(potVal, cvVal);

  if (mappedVal != currentVal)
  {
    currentVal = mappedVal;
    // this should be a float
    ad9833.ApplySignal(wf, REG0, currentVal);

    displayFrequencyNumber(currentVal);
  }

  char buttons[] = "   ";

  if (modePressed)
  {
    buttons[0] = 'M';
    //  changeSource();
  }

  if (shapePressed)
  {
    buttons[1] = 'S';
    changeWaveform();
    ad9833.ApplySignal(wf, REG0, currentVal);
  }

  if (rangePressed)
  {
    buttons[2] = 'R';
    changeRange();
  }

  displayButtonStates(buttons);
  delay(5);
}