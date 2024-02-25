#include <Arduino.h>
#include "pins.h"

int8_t encoderIncrement;
bool shortPress, longPress;

static uint8_t encPos;


void encoderISR(void) {
  static unsigned long lastEdgeDetectionTime = 0;

  if (digitalRead(ENCA_PIN)) {
    unsigned long time = micros();
    if ((time - lastEdgeDetectionTime) > 250) {
      delayMicroseconds(10);
      encPos += digitalRead(ENCB_PIN) ? -1 : +1;
    }
    lastEdgeDetectionTime = time;
  }
}

void encoderSetup() {
  pinMode(ENCA_PIN, INPUT_PULLUP);
  pinMode(ENCB_PIN, INPUT_PULLUP);
  pinMode(ENCBTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCA_PIN), encoderISR, RISING);
}

void encoderHandler(void) {
  static uint8_t prevEncPos;

  encoderIncrement = (int8_t)(encPos - prevEncPos);
  prevEncPos += encoderIncrement;

  static bool longpressDetected;
  static unsigned long lastEvent_ms;
  static bool buttonPressed, prevEncButtonReleased;

  const unsigned long ms = millis();
  const bool prevButtonPressed = buttonPressed;

  const bool encButtonReleased = digitalRead(ENCBTN_PIN);
  if (encButtonReleased ^ prevEncButtonReleased)
    lastEvent_ms = ms;
  buttonPressed = !encButtonReleased && (ms - lastEvent_ms >= 50);
  prevEncButtonReleased = encButtonReleased;

  shortPress = !buttonPressed && prevButtonPressed && !longpressDetected;
  if (!buttonPressed || (ms - lastEvent_ms) < 1500)
    longPress = longpressDetected = false;
  else {
    longPress = !longpressDetected;
    longpressDetected = true;
  }
}
