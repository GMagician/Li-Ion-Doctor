#pragma once
#include <stdint.h>
#define ADAFRUIT_ASCII96    false
#define OPTIMIZE_I2C        false
#include <SSD1306AsciiWire.h>

#define OLED_CAPTION_ROW    0
#define OLED_VALUE1_X       0
#define OLED_VALUE2_X       62
#define OLED_UNIT2_X        (62 + 12 * 4)
#define OLED_ROW1           2
#define OLED_ROW2           4
#define OLED_STATUS_ROW     7

enum class MenuAction :uint8_t {
  pageEnter, execute, asa, pageChange, menuExit
};

typedef struct {
  void (*pageHandler)(MenuAction);
  uint8_t lastPage;
  uint8_t curPage;
} menu_t;

void guiSetup(void);
void splashScreen(void);
void eepromRestored(void);
void printCaption(const char *text);
void printCaptionInfo(int16_t cmA, int16_t dmA);
void printAligned(const unsigned int value, const uint8_t digits);
void printMeasures(void);
void printVoltage(void );
void printCurrent(void );
void printStatus(const char *text);

extern menu_t menu[];
extern uint8_t menuLevel;
extern SSD1306AsciiWire oled;
