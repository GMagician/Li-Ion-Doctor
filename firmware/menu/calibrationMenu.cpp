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
  constexpr int16_t iOffsetMinLimit = IVREF_ADC(IVREF * 1.25);
  constexpr int16_t iOffsetMaxLimit = IVREF_ADC(IVREF * 0.75);

  config.iOffset += encoderIncrement;
  if (config.iOffset < iOffsetMinLimit)
    config.iOffset = iOffsetMinLimit;
  else if (config.iOffset > iOffsetMaxLimit)
    config.iOffset = iOffsetMaxLimit;

  // Update HMI
  if (action == MenuAction::pageEnter)
    printCaption("I OFFSET CALIBRATION");
  printCurrent();
}

/**
 *  Voltage offset calibration
 */
void vOffsetCalibration(MenuAction action) {
  constexpr int16_t vOffsetMinLimit = DIODE_FVD_ADC(0.2);
  constexpr int16_t vOffsetMaxLimit = DIODE_FVD_ADC(0.5);

  config.vOffset += encoderIncrement;
  if (config.vOffset < vOffsetMinLimit)
    config.vOffset = vOffsetMinLimit;
  else if (config.vOffset > vOffsetMaxLimit)
    config.vOffset = vOffsetMaxLimit;

  // Update HMI
  if (action == MenuAction::pageEnter)
    printCaption("V OFFSET CALIBRATION");
  printVoltage();
}

/**
 *  Current offset calibration
 */
void iGainCalibration(MenuAction action) {
  const float iGainMinLimit = IGAIN * 1.25;
  const float iGainMaxLimit = IGAIN * 0.75;

  config.iGain += (float)encoderIncrement / 100;
  if (config.iGain < iGainMinLimit)
    config.iGain = iGainMinLimit;
  else if (config.iGain > iGainMaxLimit)
    config.iGain = iGainMaxLimit;

  // Update HMI
  if (action == MenuAction::pageEnter)
    printCaption("I GAIN CALIBRATION");
  printCurrent();
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

    case MenuAction::pageChange:
      switch (response) {
        case UserResponse::retry:
          // Nothing to do
          break;

        case UserResponse::save:
          saveConfiguration();
          break;

        case UserResponse::reload:
          loadConfiguration();
          break;

        case UserResponse::factory:
          loadfactoryConfiguration();
          break;
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
    case UserResponse::retry:
      oled.print("RETRY  ");
      break;

    case UserResponse::save:
      oled.print("SAVE   ");
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
