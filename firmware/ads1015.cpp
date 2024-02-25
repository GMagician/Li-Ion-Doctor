#include <Arduino.h>
#include <Wire.h>
#include "common.h"


#define ADC_I2C_ADDRESS   0b1001000   // 0x48
#define ADC_V_CH          0
#define ADC_I_CH          1

enum ADS1015_Register :uint8_t { ADC_CONVERSION_REG, ADC_CONFIG_REG };
union ADS1015_Config {
  struct {
    uint16_t COMP_QUE:2;
    bool COMP_LAT:1;
    uint16_t COMP_POL:1;
    uint16_t COMP_MODE:1;
    uint16_t DR:3;
    uint16_t MODE:1;
    uint16_t PGA:3;
    uint16_t MUX:3;
    bool OS:1;
  };
  struct {
    uint8_t loByte;
    uint8_t hiByte;
  };
  uint16_t word;
};


/**
 * Get adc sampled value from specified channel
 */
int16_t adsRead(uint8_t ch) {
  ADS1015_Config cnfRegValue = {
    {
    .COMP_QUE = 0b11,               // N/U
    .COMP_LAT = 0,                  // N/U
    .COMP_POL = 0,                  // N/U
    .COMP_MODE = 0,                 // N/U
    .DR = 0b010,                    // 490SPS
    .MODE = 1,                      // Single shot
    .PGA = 0b000,                   // FSR 6.144V
    .MUX = (uint8_t)(0b100 + ch),   // AINp=ch AINn=GND
    .OS = 1                         // Start conversion
    }
  };

  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CONFIG_REG);
  Wire.write(cnfRegValue.hiByte);
  Wire.write(cnfRegValue.loByte);
  Wire.endTransmission();

  do {
    if (Wire.requestFrom(ADC_I2C_ADDRESS, 2) == 2) {
      cnfRegValue.hiByte = Wire.read();
      cnfRegValue.loByte = Wire.read();
    }
  } while (cnfRegValue.OS);

  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CONVERSION_REG);
  Wire.endTransmission();

  delay((2 * 1000 + (490 - 1)) / 490);    // 490 SPS
  while(Wire.requestFrom(ADC_I2C_ADDRESS, 2) != 2)
    ;

  return ((Wire.read() << 8) | Wire.read());
}

/**
 * Read measured voltage and current
 */
void readMeasures(void) {
  const float adcVolt = ((ADC_FSR) * (adsRead(ADC_V_CH) + config.vOffset) / (ADC_FULLSCALE));
  const float vShunt = (ADC_FSR) * (adsRead(ADC_I_CH) - config.iOffset) / (ADC_FULLSCALE) / config.iGain;

  volt = adcVolt + vShunt;
  mA = (int16_t)(vShunt * 1000 / (RSHUNT));
}
