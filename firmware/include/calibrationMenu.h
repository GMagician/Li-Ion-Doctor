#pragma once

#include <stdint.h>
#include "gui.h"


enum class Calibration :uint8_t {
  iOffset, vOffset, iGain, userResponse,
  last = userResponse
};

void calibrationHandler(MenuAction action);
