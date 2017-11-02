
#include <Arduino.h>
#include "RotaryEncoder.h"



void primarySensorFired() {

  if (digitalRead(secondSensor) == HIGH) {
    rotaryCounter -= 1;
  } else {
    rotaryCounter += 1;
  }

  newData = 1;
}

RotaryEncoder::RotaryEncoder()
{

  pinMode(firstSensor, INPUT);
  pinMode(secondSensor, INPUT);
  attachInterrupt(firstSensor, primarySensorFired, RISING);

}

void RotaryEncoder::service()
{

  static uint32_t lastRPMMeasurement = 0;
  static int32_t lastCounterValue = 0;
  static int16_t countsSinceLastMeasure = 0;
  static int32_t timeIntervalSinceLastMeasure = 0;
  if (lastRPMMeasurement + RPMUpdateIntervalInMS < millis()) {
    timeIntervalSinceLastMeasure =  millis() - lastRPMMeasurement;
    lastRPMMeasurement = millis();

    countsSinceLastMeasure = rotaryCounter - lastCounterValue;
    degreeSinceLastMeasure = (countsSinceLastMeasure / stepsPerDegree);
    degreePerMinute = degreeSinceLastMeasure * (oneMinuteInMillis / timeIntervalSinceLastMeasure);
    revolutionsPerMinute = (degreePerMinute / 360.0);
    lastCounterValue = rotaryCounter;

  }

}
