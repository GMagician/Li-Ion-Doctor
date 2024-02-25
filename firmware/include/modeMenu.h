#pragma once

#include <stdint.h>
#include <gui.h>


enum class Mode :uint8_t {
  health, capacity, charge,
 #if HAS_RECOVERY
   recovery, 
 #endif
 discharge,
  last = discharge
};

void modeSetup(void);
void modeHandler(MenuAction action);
