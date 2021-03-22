#include <Arduino.h>
#include <SPI.h>
#include <AD9833.h>
#include <display.h>
#include <SimpleKalmanFilter.h>
#include <math.h>
#include <avdweb_Switch.h>
#include <FrequencyTimer2.h> // might not be used

// Pins that we need to keep track of
#define FSYNC 10

#define MODE 4  // press for knob/knob+CV, long press for display on/off
#define SHAPE 3 // press to cycle through waveforms
#define RANGE 2 // press to cycle through ranges, long press to go to 1V/Oct

#define KNOB A0
#define CV A1
typedef enum
{
  LFO1 = 0,
  LFO2 = 1,
  LIN1 = 2,
  LIN2 = 3,
  VOCT1 = 4,
  VOCT2 = 5
} Range;

bool updateRangeText = true;

Switch btnMode = Switch(MODE);
Switch btnShape = Switch(SHAPE);
Switch btnRange = Switch(RANGE);

AD9833 ad9833(FSYNC);
WaveformType wf = SINE_WAVE;

bool vPerOct = false;
bool displayOn = true;
bool cvActive = false;
Range range = LFO1;

const float stepsPerVolt = 204.6;
const float C2freq = 65.40639133;
float multiplier = 0.01;

// filtering for potentiometer - NOT CV!
SimpleKalmanFilter filter(2, 2, 0.01);

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
}

void toggleCV()
{
  cvActive = !cvActive;
  displayCVmode(cvActive);
  updateRangeText = true;
}

void changeMapping()
{
  if (vPerOct)
  {
    vPerOct = false;
    range = LFO1;
    multiplier = 0.01;
  }
  else
  {
    vPerOct = true;
    range = VOCT1;
    multiplier = 1;
  }
  updateRangeText = true;
}

void toggleDisplay()
{
  if (displayOn)
  {
    displayOn = false;
    blankFrequency();
  }
  else
  {
    displayOn = true;
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float calculateFrequency(int potVal, int cvVal)
{
  // handle the filtering of inputs
  float filtered = filter.updateEstimate(potVal); // need to do CV soon
  float potFreq, cvFreq;

  if (!cvActive)
  {
    cvVal = 0;
  }

  if (vPerOct)
  {
    // do a volt-per-octave thing. one of the ranges is a +/- 1V if there's
    // a control voltage, 0-2v if not
    cvFreq = cvActive ? (pow(2, cvVal / stepsPerVolt) * C2freq) : 0;

    if (range == VOCT1)
    {
      potFreq = (pow(2, filtered / stepsPerVolt) * C2freq);
    }
    else
    {
      if (cvActive)
      {
        potFreq = mapfloat(filtered, 0, 1023, -200, 200);
      }
      else
      {
        filtered = (filtered / 2.5);
        potFreq = (pow(2, filtered / stepsPerVolt) * C2freq);
      }
    }

    return max(0, potFreq + cvFreq); // clamp it positive
  }
  else
  {
    // do a normal LFO thing
    potFreq = mapfloat(filtered, 0, 1023, 0, 100);
    cvFreq = mapfloat(cvVal, 0, 1023, 0, 100);

    // do some multiplying to get a value. not sure if the cv should be treated in the
    // same way as the pot but given it's not a 1V/Oct it's reasonable

    return (potFreq + cvFreq) * multiplier;
  }
}

void setup()
{
  //Serial.begin(9600);
  ad9833.Begin(); // apparently HAS to be the first command

  setupDisplay();

  filter.updateEstimate(analogRead(KNOB));

  ad9833.ApplySignal(wf, REG0, 0);
  ad9833.EnableOutput(true);
}

void loop()
{
  // get input
  int potVal = 1023 - analogRead(KNOB); // because I wired the pot up the wrong way round :|
  int cvVal = analogRead(CV);

  // process input
  float mappedVal = calculateFrequency(potVal, cvVal);

  if (mappedVal != currentVal)
  {
    currentVal = mappedVal;
    ad9833.ApplySignal(wf, REG0, currentVal);

    if (displayOn)
    {
      displayFrequencyNumber(currentVal, multiplier);
    }
  }

  btnMode.poll();
  if (btnMode.singleClick())
  {
    // toggle between just knob and knob/cv added
    toggleCV();
  }
  else if (btnMode.longPress())
  {
    // toggle whether or not the display gets used
    toggleDisplay();
  }

  btnShape.poll();
  if (btnShape.singleClick())
  {
    // cycle through waveforms
    changeWaveform();
    ad9833.ApplySignal(wf, REG0, currentVal);
  }

  btnRange.poll();
  if (btnRange.singleClick())
  {
    // if linear, cycle through some LFO ranges
    if (!vPerOct)
    {
      if (range == LFO1)
      {
        range = LFO2;
        multiplier = 0.1;
      }
      else if (range == LFO2)
      {
        range = LIN1;
        multiplier = 1;
      }
      else if (range == LIN1)
      {
        range = LIN2;
        multiplier = 10;
      }
      else
      {
        range = LFO1;
        multiplier = 0.01;
      }
    }
    else
    {
      range = (range == VOCT1) ? VOCT2 : VOCT1;
    }
    updateRangeText = true;
  }
  else if (btnRange.longPress())
  {
    // swap from linear to 1V/Oct
    changeMapping();
  }

  if (updateRangeText)
  {
    updateRangeText = false;
    if (range == LFO1)
    {
      displayRange("LFO 0 - 1 Hz    ");
    }
    else if (range == LFO2)
    {
      displayRange("LFO 0 - 10 Hz   ");
    }
    else if (range == LIN1)
    {
      displayRange("Lin 0 - 100 Hz  ");
    }
    else if (range == LIN2)
    {
      displayRange("Lin 0 - 1000 Hz ");
    }
    else if (range == VOCT1)
    {
      displayRange("Volt/Octave, +5V");
    }
    else if (range == VOCT2)
    {
      if (cvActive)
      {
        displayRange("1V/Oct fine tune");
      }
      else
      {
        displayRange("2 Octave range  ");
      }
    }
  }

  delay(1);
}