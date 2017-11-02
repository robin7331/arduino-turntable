#ifndef MotorControl_h
#define MotorControl_h

#include "Arduino.h"

#define DIRECTION_NONE 0
#define DIRECTION_CW 1
#define DIRECTION_CCW 2

class MotorControl
{
  public:
    MotorControl(uint8_t pwmPin, uint8_t AIN1Pin, uint8_t AIN2Pin);

    uint8_t currentPower, currentDirection;
    void apply(uint8_t power, uint8_t direction);
    void stop();

  private:
    uint8_t _pwmPin, _AIN1Pin, _AIN2Pin;

};

#endif
