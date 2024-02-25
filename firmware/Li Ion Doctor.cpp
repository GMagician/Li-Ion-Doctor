/**
 * Lithium battery doctor board
 *
 * Copyright(C) 2022-2024 Akos Boda/GMagician
 * Original code by Boda, modified by GMagician
 *
 */
#include <Arduino.h>
#include <avr/wdt.h>
#include "pins.h"
#include "ads1015.h"
#include "encoder.h"
#include "common.h"
#include "gui.h"
#include "modeMenu.h"


/**
 *  Hardware initialization
 */
void setup() {
  wdt_disable();

  modeSetup();
  encoderSetup();

  guiSetup();
  
  if (loadConfiguration()) {
    // Invalid CRC, restore eeprom defaults
    loadfactoryConfiguration();
    eepromRestored();
    for(;;);
  }

  splashScreen();
  menu[menuLevel].pageHandler(MenuAction::pageEnter);

  wdt_enable(WDTO_4S);
}

/**
 *  Main loop
 */
void loop() {
  wdt_reset();
  encoderHandler();

  readMeasures();
  batteryPresent = (volt >= 2.35);

  if (!shortPress && !longPress)
    menu[menuLevel].pageHandler(MenuAction::execute);
  if (shortPress || longPress) {
    if (longPress) {
      menu[menuLevel].pageHandler(MenuAction::menuExit);
      menuLevel = 1 - menuLevel;
      menu[menuLevel].curPage = 0;
    } else {
      menu[menuLevel].pageHandler(MenuAction::pageChange);
      if (menu[menuLevel].curPage >= menu[menuLevel].lastPage)
        menu[menuLevel].curPage = 0;
      else
        menu[menuLevel].curPage = (menu[menuLevel].curPage % menu[menuLevel].lastPage) + 1;
    }
    oled.clear();
    menu[menuLevel].pageHandler(MenuAction::pageEnter);
  }
}
