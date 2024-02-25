/**
 * Lithium battery doctor board
 *
 * Copyright(C) 2022-2024 Akos Boda/GMagician
 * Original code by Boda, modified by GMagician
 *
 */
#include <Arduino.h>
#include <avr/wdt.h>
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
  
  if (!loadConfiguration())
    splashScreen();
  else {
    // Invalid CRC, restore eeprom defaults
    reloadFactoryConfiguration();
    printEEpromCorrupted();
    menuLevel = 1;
    delay(2500);
    oled.clear();
  }
  wdt_enable(WDTO_500MS);

  menu[menuLevel].pageHandler(MenuAction::pageEnter);
}

/**
 *  Main loop
 */
void loop() {
  wdt_reset();
  encoderHandler();

  readMeasures();
  batteryPresent = !(volt > 1.8 && volt < 2.1);

  if (!shortPress && !longPress)
    menu[menuLevel].pageHandler(MenuAction::execute);
  else {
    if (longPress) {
      menu[menuLevel].pageHandler(MenuAction::menuExit);
      menuLevel = 1 - menuLevel;
      menu[menuLevel].curPage = 0;
    } else {
      menu[menuLevel].pageHandler(MenuAction::pageExit);
      if (menu[menuLevel].curPage >= menu[menuLevel].lastPage)
        menu[menuLevel].curPage = 0;
      else
        menu[menuLevel].curPage = (menu[menuLevel].curPage % menu[menuLevel].lastPage) + 1;
    }
    oled.clear();
    menu[menuLevel].pageHandler(MenuAction::pageEnter);
  }
}
