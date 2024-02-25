#include <Arduino.h>
#include <avr/wdt.h>
#include "common.h"
#include "gui.h"
#include "modeMenu.h"
#include "calibrationMenu.h"


#define OLED_I2C_ADDRESS    0b0111100   // 0x3C

SSD1306AsciiWire oled;
uint8_t menuLevel = 0;
menu_t menu[] = {
  { &modeHandler, (uint8_t)Mode::last },
  { &calibrationHandler, (uint8_t)Calibration::last }
};


/**
 * Gui setup
 */
void guiSetup() {
  // Initialize I2C
  Wire.begin();
  Wire.setClock(400000);

  // Initialize oled
  oled.begin(&Adafruit128x64, OLED_I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  }

/**
 * Display splash screen
 */
void splashScreen() {
  oled.set1X();
  oled.setCursor(25, 2);
  oled.print("Li-Ion Doctor");
  oled.setCursor(49, 4);
  oled.print("V2.00");
  delay(1000);
  oled.clear();
}

/**
 * Display eeprom corrupted page
 */
void printEEpromCorrupted() {
  oled.set2X();
  oled.setCursor(28, 0);
  oled.print("EEPROM");
  oled.setCursor(10, 2);
  oled.print("CORRUPTED");
  oled.set1X();
  oled.setCursor(13, 5);
  oled.print("RECALIB. REQUIRED");
}

/**
 * Print caption
 */
void printCaption(const char *text) {
  oled.set1X();
  oled.setCursor(0, OLED_CAPTION_ROW);
  oled.setInvertMode(true);
  oled.print(text);
  while (oled.col() < oled.displayWidth())
    oled.print(" ");
  oled.setInvertMode(false);
}

/**
 * Print caption additional info
 */
void printCaptionInfo(int16_t cmA, int16_t dmA) {
  const bool displayBoth = (cmA > 0 && dmA > 0);

  oled.set1X();
  oled.setCursor(oled.displayWidth() - (displayBoth ? 11 : 6) * 6, OLED_CAPTION_ROW);
  oled.setInvertMode(true);
  if (cmA > 0)
    printAligned(cmA, 4);
  if (displayBoth)
    oled.print('/');
  if (dmA > 0)
    printAligned(dmA, 4);
  oled.print("mA");
  oled.setInvertMode(false);
}

/**
 * Print value aligned to specified digits
 */
void printAligned(const int16_t value, const uint8_t digits) {
  if (value > -100 && value < 1000 && digits >= 4)
    oled.print(' ');
  if (value > -10 && value < 100 && digits >= 3)
    oled.print(' ');
  if (value > -1 && value < 10 && digits >= 2)
    oled.print(' ');
  oled.print(value);
}


/**
 * Print voltage and current
 */
void printMeasures() {
  if (batteryPresent && volt > 0 && volt <= 9.99)
    printVoltage();
  else {
    oled.set2X();
    oled.setCursor(OLED_VALUE1_X, OLED_ROW1);
    oled.print("----");
    oled.set1X();
    oled.setCursor(OLED_UNIT2_X, OLED_ROW1 + 1);
    oled.print("V");
    }
  int16_t absmA = abs(mA);
  if (absmA >= 5 && absmA <= 9999)
    printCurrent();
  else {
    oled.set1X();
    oled.setCursor(OLED_UNIT2_X, OLED_ROW1 + 1);
    oled.print("mA");
  }
}

/**
 * Print voltage
 */
void printVoltage() {
  oled.set2X();
  oled.setCursor(OLED_VALUE1_X, OLED_ROW1);
  oled.print(volt, 2);
  oled.set1X();
  oled.setRow(OLED_ROW1 + 1);
  oled.print("V");
}

/**
 * Print current
 */
void printCurrent(bool showAbs) {
  oled.set2X();
  oled.setCursor(OLED_VALUE2_X, OLED_ROW1);
  printAligned((showAbs && mA < 0) ? -mA : mA , 4);
  oled.set1X();
  oled.setRow(OLED_ROW1 + 1);
  oled.print("mA");
}

/**
 * Print status message
 */
void printStatus(const char *text) {
  oled.set1X();
  oled.setCursor(0, OLED_STATUS_ROW);
  oled.print(text);
  oled.clearToEOL();
}
