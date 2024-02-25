#include <SSD1306AsciiWire.h>
#include <EEPROM.h>
#include <common.h>


float volt;
int16_t mA;
bool batteryPresent;
configuration_t config;

static const configuration_t PROGMEM defaultConfig = {
  .iOffset = IVREF_ADC(IVREF),
  .vOffset = DIODE_FVD_ADC(0.37),
  .iGain = IGAIN
};


/**
 *  Restore eeprom default configuration
 */
void loadfactoryConfiguration() {
  uint8_t* configPtr = (uint8_t*)&config;
  const uint8_t* defConfigPtr = (const uint8_t*)&defaultConfig;
  for (uint8_t eeAddr = 0; eeAddr < sizeof(config); ++eeAddr, ++configPtr, ++defConfigPtr)
    *configPtr = pgm_read_byte(defConfigPtr);
  saveConfiguration();
}

/**
 *  Load configuration from eeprom
 */
bool loadConfiguration() {
  uint8_t crc = 0x00;
  uint8_t* configPtr = (uint8_t*)&config;
  for (uint8_t eeAddr = 0; eeAddr < sizeof(config); ++eeAddr, ++configPtr) {
    *configPtr = EEPROM.read(eeAddr);
    crc ^= *configPtr;
  }

  return crc ^ EEPROM.read(sizeof(config));
}

/**
 *  Save configuration to eeprom
 */
void saveConfiguration() {
  uint8_t crc = 0x00;
  uint8_t* configPtr = (uint8_t*)&config;
  for (uint8_t eeAddr = 0; eeAddr < sizeof(config); ++eeAddr, ++configPtr) {
    EEPROM.write(eeAddr, *configPtr);
    crc ^= *configPtr;
  }
  EEPROM.write(sizeof(config), crc);
}
