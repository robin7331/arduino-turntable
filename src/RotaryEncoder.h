#ifndef RotaryEncoder_H
#define RotaryEncoder_H

#include "Arduino.h"

#define stepsPerFullRotation 18304.0
#define stepsPerDegree (stepsPerFullRotation/360.0)
#define oneMinuteInMillis (60.0 * 1000.0)
#define RPMUpdateIntervalInMS 250.0

#define firstSensor 7
#define secondSensor 6
static uint8_t newData = 1;
static int32_t rotaryCounter = 0;

class RotaryEncoder
{
  public:
    RotaryEncoder();
    void service();
    double degreeSinceLastMeasure = 0;
    double degreePerMinute = 0;
    double revolutionsPerMinute = 0;

};

#endif
