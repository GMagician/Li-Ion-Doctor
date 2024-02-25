#include <avr/wdt.h>
#include "common.h"
#include "encoder.h"
#include "include/calibrationMenu.h"


enum class UserResponse :uint8_t {
  first = 0,
  retry = first, save, reload, factory,
  last = factory
};


/**
 *  Current offset calibration
 */
void iOffsetCalibration(MenuAction action) {
  constexpr int16_t iOffsetMinLimit = IVREF_ADC(IVREF * 0.75);
  constexpr int16_t iOffsetMaxLimit = IVREF_ADC(IVREF * 1.25);
  constexpr int16_t iOffsetStep = IVREF_ADC(0.0001);

  config.iOffset -= (iOffsetStep * encoderIncrement);
  if (config.iOffset < iOffsetMinLimit)
    config.iOffset = iOffsetMinLimit;
  else if (config.iOffset > iOffsetMaxLimit)
    config.iOffset = iOffsetMaxLimit;

  if (action == MenuAction::pageEnter)
    printCaption("I OFFSET CALIBRATION");

  // Update HMI
  printCurrent(false);
  oled.set1X();
  oled.setCursor(0, OLED_STATUS_ROW);
  oled.print("Ref.: ");
  const float offset = (float)config.iOffset / IVREF_ADC(1);
  oled.print(offset, 4);
  oled.print("V");
}

/**
 *  Voltage offset calibration
 */
void vOffsetCalibration(MenuAction action) {
  constexpr int16_t vOffsetMinLimit = DIODE_FVD_ADC(0.001);
  constexpr int16_t vOffsetMaxLimit = DIODE_FVD_ADC(0.370);
  constexpr int16_t vVoltStep = DIODE_FVD_ADC(0.001);

  config.vOffset += (vVoltStep * encoderIncrement);
  if (config.vOffset < vOffsetMinLimit)
    config.vOffset = vOffsetMinLimit;
  else if (config.vOffset > vOffsetMaxLimit)
    config.vOffset = vOffsetMaxLimit;

  if (action == MenuAction::pageEnter)
    printCaption("V OFFSET CALIBRATION");

  // Update HMI
  printVoltage();
  oled.set1X();
  oled.setCursor(0, OLED_STATUS_ROW);
  oled.print("Drop: ");
  const float offset = (float)config.vOffset / DIODE_FVD_ADC(1);
  oled.print(offset, 3);
  oled.print("V");
}

/**
 *  Current gain calibration
 */
void iGainCalibration(MenuAction action) {
  static int16_t orgDischarge_mA;
  constexpr float iGainMinLimit = IGAIN * 0.75;
  constexpr float iGainMaxLimit = IGAIN * 1.25;

  config.iGain += (0.05 * encoderIncrement);
  if (config.iGain < iGainMinLimit)
    config.iGain = iGainMinLimit;
  else if (config.iGain > iGainMaxLimit)
    config.iGain = iGainMaxLimit;

  if (action == MenuAction::pageEnter) {
    orgDischarge_mA = discharge_mA;
    discharge_mA = 200;
    printCaption("I GAIN CALIBRATION");
    printCaptionInfo(-1, discharge_mA);
    EnableDischargeCurrent();
  } else if (action == MenuAction::pageExit || action == MenuAction::menuExit)
    discharge_mA = orgDischarge_mA;

  // Update HMI
  printCurrent();
  oled.set1X();
  oled.setCursor(0, OLED_STATUS_ROW);
  oled.print("Gain: ");
  oled.print(config.iGain, 3);
}

/**
 *  Get user response on end of calibration
 */
void getCalibrationUserResponse(MenuAction action) {
  static UserResponse response;

  switch (action) {
    case MenuAction::pageEnter:
      response = UserResponse::retry;
      break;

    case MenuAction::pageExit:
      if (response == UserResponse::factory) {
        reloadFactoryConfiguration();
        saveConfiguration();
      } else if (response == UserResponse::save)
        saveConfiguration();
      if (response != UserResponse::retry) {
        wdt_enable(WDTO_15MS);
        for(;;);    // Reboot
      }
      break;

    default:
      const int8_t newResponse = (int8_t)response + encoderIncrement;
      if (newResponse < (int8_t)UserResponse::first)
        response = UserResponse::last;
      else if (newResponse > (int8_t)UserResponse::last)
        response = UserResponse::first;
      else
        response = (UserResponse)newResponse;
  }

  // Update HMI
  if (action == MenuAction::pageEnter)
    printCaption("CALIBRATED");
  oled.set1X();
  oled.setCursor(0, 3);
  oled.print("Action requested:");
  oled.setCursor(86, 4);
  oled.setInvertMode(true);
  switch (response) {
    case UserResponse::save:
      oled.print("SAVE   ");
      break;

    case UserResponse::retry:
      oled.print("RETRY  ");
      break;

    case UserResponse::reload:
      oled.print("RELOAD ");
      break;

    case UserResponse::factory:
      oled.print("FACTORY");
      break;
  }
  oled.setInvertMode(false);
}

/**
 * Calibration menu level handling
 */ 
void calibrationHandler(MenuAction action) {
  switch ((Calibration)menu[menuLevel].curPage) {
    case Calibration::iOffset:
      iOffsetCalibration(action);
      break;

    case Calibration::vOffset:
      vOffsetCalibration(action);
      break;

    case Calibration::iGain:
      iGainCalibration(action);
      break;

    case Calibration::userResponse:
      getCalibrationUserResponse(action);
      break;
  }
}
