
#import <Arduino.h>
#import "MotorControl.h"

MotorControl::MotorControl(uint8_t pwmPin, uint8_t AIN1Pin, uint8_t AIN2Pin)
{
  _pwmPin = pwmPin;
  _AIN1Pin = AIN1Pin;
  _AIN2Pin = AIN2Pin;

  currentPower = 0;
  currentDirection = DIRECTION_NONE;

  pinMode(_AIN1Pin, OUTPUT);
  pinMode(_AIN2Pin, OUTPUT);

}

void MotorControl::apply(uint8_t power, uint8_t direction) {


  if (currentPower != power) {
    Serial.print("Applying Power: ");
    Serial.println(power);
     analogWrite(_pwmPin, power);
     currentPower = power;
  }

  if (currentDirection != direction) {
    if (direction == DIRECTION_CW) {
      digitalWrite(_AIN1Pin, HIGH);
      digitalWrite(_AIN2Pin, LOW);
      Serial.println("CW");
    } else if (direction == DIRECTION_CCW) {
      digitalWrite(_AIN1Pin, LOW);
      digitalWrite(_AIN2Pin, HIGH);
      Serial.println("CCW");
    } else {
      digitalWrite(_AIN1Pin, LOW);
      digitalWrite(_AIN2Pin, LOW);
      Serial.println("NONE");
    }

    currentDirection = direction;
  }

}

void MotorControl::stop() {
  apply(0, DIRECTION_NONE);
}
