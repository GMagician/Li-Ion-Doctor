#include <Wire.h>
#include "common.h"


#define ADC_I2C_ADDRESS   0b1001000   // 0x48
#define ADC_V_CH          0
#define ADC_I_CH          1

enum ADS1015_Register :uint8_t { ADC_CONVERSION_REG, ADC_CONFIG_REG };
union ADS1015_Config {
  struct {
    bool OS:1;
    uint16_t MUX:3;
    uint16_t PGA:3;
    uint16_t MODE:1;
    uint16_t DR:3;
    uint16_t COMP_MODE:1;
    uint16_t COMP_POL:1;
    bool COMP_LAT:1;
    uint16_t COMP_QUE:2;
  };
  struct {
    uint8_t hiByte;
    uint8_t loByte;
  };
};


/**
 * Get adc sampled value from specified channel
 */
int16_t adsRead(uint8_t ch) {
return 0; // HACK
  ADS1015_Config cnfRegValue = {
    {
    .OS = 1,                        // Start conversion
    .MUX = (uint8_t)(0b100 + ch),   // AINp=ch AINn=GND
    .PGA = 0,                       // FSR 6.144V
    .MODE = 1,                      // Single shot
    .DR = 0b010,                    // 490SPS
    .COMP_MODE = 0,                 // N/U
    .COMP_POL = 0,                  // N/U
    .COMP_LAT = 0,                  // N/U
    .COMP_QUE = 0b11                // N/U
    }
  };

  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CONFIG_REG);
  Wire.write(cnfRegValue.hiByte);
  Wire.write(cnfRegValue.loByte);
  Wire.endTransmission();

  do {
    if (Wire.requestFrom(ADC_I2C_ADDRESS, 2, ADC_CONFIG_REG, 1, true) == 2) {
      cnfRegValue.hiByte = Wire.read();
      cnfRegValue.loByte = Wire.read();
    }
  } while (cnfRegValue.OS);

  while (Wire.requestFrom(ADC_I2C_ADDRESS, 2, ADC_CONVERSION_REG, 1, true) != 2)
    ;

  return ((Wire.read() << 8) | Wire.read()) / 8;
}

/**
 * Read measured voltage and current
 */
void readMeasures(void) {
  const int16_t adcValue = adsRead(ADC_I_CH) - config.iOffset;
  const float vShunt = (ADC_FSR) * adcValue / (ADC_FULLSCALE) / config.iGain;

  volt = ((ADC_FSR) * (adsRead(ADC_V_CH) + config.vOffset) / (ADC_FULLSCALE)) + vShunt;
  mA = (uint16_t)(abs(vShunt) * 1000 / (RSHUNT));
}
