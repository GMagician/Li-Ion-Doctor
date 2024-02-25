#pragma once

#include <stdint.h>

extern int8_t encoderIncrement;
extern bool shortPress, longPress;

void encoderSetup();
void encoderHandler(void);
