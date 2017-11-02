#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ClickEncoder.h>
#include "MotorControl.h"
#include "RotaryEncoder.h"



#define EEPROM_DID_WRITE 0
#define EEPROM_LAST_ROTARY_VALUE 1
#define EEPROM_INVERT_ROTARY_VALUE 2

// Encoder Pins
#define ENCODER_PINA     4
#define ENCODER_PINB     2
#define ENCODER_BTN      3
#define ENCODER_STEPS_PER_NOTCH    4   // Change this depending on which encoder is used

// MotorControl Pins
#define PWM_PIN 11
#define AIN1_PIN 12
#define AIN2_PIN 10

// State Management
#define HALTED 0
#define RUNNING 1

// UI Constants
#define TOP_MARGIN 10
#define LARGE_INDICATOR_HEIGHT 3
#define SMALL_INDICATOR_HEIGHT 2
#define ZERO_INDICATOR_HEIGHT 3
#define BAR_HEIGHT 6
#define BAR_MARGIN_TOP 2
#define BAR_MARGIN_BOTTOM 4
#define BAR_WITH_MARGINS BAR_HEIGHT + BAR_MARGIN_TOP + BAR_MARGIN_BOTTOM

uint8_t currentState = HALTED;
uint8_t showStateLabel = false;
int16_t rotaryValue = 0;
uint8_t calculatedPower, calculatedDirection;
uint32_t lastUIRefresh = 0;
uint8_t UICounter = 0;

ClickEncoder *encoder;
MotorControl *motorControl;
Adafruit_SSD1306 *display;
RotaryEncoder *rotaryEncoder;

void setup() {

  Serial.begin(115200);
  while(!Serial) {};

  encoder = new ClickEncoder(ENCODER_PINA,ENCODER_PINB,ENCODER_BTN,ENCODER_STEPS_PER_NOTCH);
  encoder->setAccelerationEnabled(true);

  motorControl = new MotorControl(PWM_PIN, AIN1_PIN, AIN2_PIN);

  display = new Adafruit_SSD1306();
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display->begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display->clearDisplay();

  rotaryEncoder = new RotaryEncoder();



  if (EEPROM.read(EEPROM_DID_WRITE) == 255) {
    EEPROM.write(EEPROM_DID_WRITE, 0);
    EEPROM.write(EEPROM_LAST_ROTARY_VALUE, 0);
    EEPROM.write(EEPROM_INVERT_ROTARY_VALUE, 0);
  } else {
    rotaryValue = EEPROM.read(EEPROM_LAST_ROTARY_VALUE);
    if (EEPROM.read(EEPROM_INVERT_ROTARY_VALUE) == 1) {
      rotaryValue = -rotaryValue;
    }

    Serial.print("Reading EEPROM ");
    Serial.print(rotaryValue);
  }

}



void drawPowerBar() {

  if (calculatedDirection == DIRECTION_CW) {
    int8_t width = map(calculatedPower, 0, 255, 1, 64);
    display->fillRect(64, TOP_MARGIN + BAR_MARGIN_TOP, width, BAR_HEIGHT, WHITE);
  } else if (calculatedDirection == DIRECTION_CCW) {
    int8_t width = map(calculatedPower, 0, 255, 1, 64);
    display->fillRect(64 - width, TOP_MARGIN + BAR_MARGIN_TOP, width + 1, BAR_HEIGHT, WHITE);
  } else {
    display->fillRect(64, TOP_MARGIN + BAR_MARGIN_TOP, 1, BAR_HEIGHT, WHITE);
  }

}

void drawUI() {
  display->clearDisplay();

  // Draw power scale
  // Top and bottom horizontal lines
  display->drawLine(0, TOP_MARGIN, 127, TOP_MARGIN, WHITE);
  display->drawLine(0, TOP_MARGIN + BAR_WITH_MARGINS, 127, TOP_MARGIN + BAR_WITH_MARGINS, WHITE);

  // Max CCW indicator lines
  display->drawLine(0, TOP_MARGIN, 0, TOP_MARGIN + LARGE_INDICATOR_HEIGHT, WHITE);
  display->drawLine(0, TOP_MARGIN + BAR_WITH_MARGINS, 0, TOP_MARGIN + BAR_WITH_MARGINS - LARGE_INDICATOR_HEIGHT, WHITE);

  // Mid CCW indicator lines
  // display->drawLine(32, TOP_MARGIN, 32, TOP_MARGIN + SMALL_INDICATOR_HEIGHT, WHITE);
  display->drawLine(32, TOP_MARGIN + BAR_WITH_MARGINS, 32, TOP_MARGIN + BAR_WITH_MARGINS - SMALL_INDICATOR_HEIGHT, WHITE);

  // Zero indicator lines
  // display->drawLine(64, TOP_MARGIN, 64, TOP_MARGIN + ZERO_INDICATOR_HEIGHT, WHITE);
  display->drawLine(64, TOP_MARGIN + BAR_WITH_MARGINS, 64, TOP_MARGIN + BAR_WITH_MARGINS - ZERO_INDICATOR_HEIGHT, WHITE);

  // Mid CW indicator lines
  // display->drawLine(96, TOP_MARGIN, 96, TOP_MARGIN + SMALL_INDICATOR_HEIGHT, WHITE);
  display->drawLine(96, TOP_MARGIN + BAR_WITH_MARGINS, 96, TOP_MARGIN + BAR_WITH_MARGINS - SMALL_INDICATOR_HEIGHT, WHITE);

  // Max CW indicator lines
  display->drawLine(127, TOP_MARGIN, 127, TOP_MARGIN + LARGE_INDICATOR_HEIGHT, WHITE);
  display->drawLine(127, TOP_MARGIN + BAR_WITH_MARGINS, 127, TOP_MARGIN + BAR_WITH_MARGINS - LARGE_INDICATOR_HEIGHT, WHITE);

  if (calculatedDirection != DIRECTION_NONE) {
    uint8_t percentage = map(calculatedPower, 0, 255, 0, 100);
    uint8_t x = 107;
    if (percentage > 10 && percentage < 100) {
      x = 105;
    } else if (percentage == 100) {
      x = 102;
    }

    display->setCursor(x, 0);
    display->setTextSize(0);
    display->print(percentage);
    display->println("%");

    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor((calculatedDirection == DIRECTION_CW) ? 59 : 57, 0);
    display->println((calculatedDirection == DIRECTION_CW) ? "CW" : "CCW");

  }


  display->setTextSize(1);
  display->setCursor(0,0);

  if (currentState == HALTED) {
    if (showStateLabel) {
      display->println("PAUSED");
    }

    if ((UICounter % 4) == 0) {
      showStateLabel = !showStateLabel;
    }

  } else {
    display->println("RUNNING");
  }

  display->setCursor(0, 25);
  display->print(rotaryEncoder->degreePerMinute / 60.0);
  display->println(" d/s");

  display->setCursor(70, 25);
  display->print(rotaryEncoder->revolutionsPerMinute);
  display->println(" rpm");

  drawPowerBar();

  display->display();
}



void setState(uint8_t state) {
  if (state != currentState) {
    currentState = state;

    if (currentState == RUNNING) {
    } else {
      showStateLabel = true;
    }

  }
}


 void calculate() {

   // cap rotary value
   if (rotaryValue < 0) {
     calculatedPower = -rotaryValue*5;
     calculatedDirection = DIRECTION_CCW;
   } else if (rotaryValue > 0) {
     calculatedPower = rotaryValue*5;
     calculatedDirection = DIRECTION_CW;
   } else {
     calculatedPower = 0;
     calculatedDirection = DIRECTION_NONE;
   }

   if (currentState == RUNNING) {
     motorControl->apply(calculatedPower, calculatedDirection);
   } else {
     motorControl->stop();
   }

   rotaryEncoder->service();

 }

void loop() {

  static uint32_t lastService = 0;

  if (lastService + 1000 < micros()) {
    lastService = micros();
    calculate();
    encoder->service();
  }

  if (millis() - lastUIRefresh > 100) {
    if (UICounter < 255) {
      UICounter ++;
    } else {
      UICounter = 0;
    }
    drawUI();
    lastUIRefresh = millis();
  }

  ClickEncoder::Button b = encoder->getButton();
  if (b == ClickEncoder::Clicked) {
    if (currentState == HALTED) {
      setState(RUNNING);
      EEPROM.write(EEPROM_INVERT_ROTARY_VALUE, (rotaryValue < 0));
      EEPROM.write(EEPROM_LAST_ROTARY_VALUE, (rotaryValue < 0) ? -rotaryValue : rotaryValue);

      Serial.print("Writing EEPROM ");
      Serial.print((rotaryValue < 0));
      Serial.print(" - ");
      Serial.println((rotaryValue < 0) ? -rotaryValue : rotaryValue);
    } else {
      setState(HALTED);
    }
    drawUI();
  }


  if (b == ClickEncoder::Held) {
    rotaryValue = 0;
    setState(HALTED);
  }


  rotaryValue += -(encoder->getValue());
  if (rotaryValue > 51) {
    rotaryValue = 51;
  }
  if (rotaryValue < -51) {
    rotaryValue = -51;
  }

    // put your main code here, to run repeatedly:
}
