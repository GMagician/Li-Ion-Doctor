#pragma once
#include <stdint.h>
#include "ads1015.h"

#define RSHUNT              0.05                          // R17
#define NIVD                ((float)1000 / (100 + 1000))  // R13 / (R11 + R13)
#define r1i                 27                            // R12
#define r2i                 620                           // R10
#define IVD                 ((float)r1i / (r1i + r2i))
#define IVREF               1.1
#define MAX_DISCHARGE_mA    2000

#define DIODE_FVD_ADC(v)    (int16_t)(((ADC_FULLSCALE) * (v) / (ADC_FSR)) + 0.5)
#define IVREF_ADC(v)        (int16_t)(((ADC_FULLSCALE) * (v) / (ADC_FSR) * (NIVD) / (IVD)) + 0.5)
#define IGAIN               ((float)r2i / r1i)

// EEPROM mapping
typedef struct {
  int16_t iOffset;
  int16_t vOffset;
  float iGain;
} configuration_t;

void loadfactoryConfiguration();
bool loadConfiguration(void);
void saveConfiguration(void);
void readMeasures();

extern float volt;
extern int16_t mA;
extern bool batteryPresent;
extern configuration_t config;
