#pragma once

#include <stdint.h>
#include "pins.h"
#include "ads1015.h"

#define RSHUNT              0.05                          // R10
#define NIVD                ((float)100 / (100 + 1000))   // R12 / (R12 + R11)
#define r1i                 36                            // R8
#define r2i                 620                           // R7
#define IVD                 ((float)r1i / (r1i + r2i))
#define IVREF               1.1
#define MIN_DISCHARGE_mA    100
#define MAX_DISCHARGE_mA    2000

#define VCC                 5.0
#define rp1                 (20000 + 12000)               // R14 + R15
#define rp2                 (820 + RSHUNT)                // R13 + RSHUNT
#define PWM_VALUE           ((RSHUNT) * discharge_mA * ((rp1) + (rp2)) / (rp2) * 255 / (VCC))

#define DIODE_FVD_ADC(v)    (int16_t)(((ADC_FULLSCALE) * (v) / (ADC_FSR)) + 0.5)
#define IVREF_ADC(v)        (int16_t)(((ADC_FULLSCALE) * (v) / (ADC_FSR) * (NIVD) / (IVD)) + 0.5)
#define IGAIN               ((float)r2i / r1i)

#define DisableDischargeCurrent() \
  do{ \
    analogWrite(DISCHARGE_PWM_PIN, 0); \
    digitalWrite(DISCHARGE_EN_PIN, LOW); \
    }while(0)
#define EnableDischargeCurrent() \
  do{ \
    analogWrite(DISCHARGE_PWM_PIN, PWM_VALUE); \
    digitalWrite(DISCHARGE_EN_PIN, HIGH); \
    }while(0)

// EEPROM mapping
typedef struct {
  int16_t iOffset;
  int16_t vOffset;
  float iGain;
} configuration_t;

void reloadFactoryConfiguration();
bool loadConfiguration(void);
void saveConfiguration(void);
void readMeasures();

extern float volt;
extern int16_t mA;
extern bool batteryPresent;
extern configuration_t config;
extern int16_t charge_mA, discharge_mA;
