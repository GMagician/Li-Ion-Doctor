#include "common.h"
#include "encoder.h"
#include "./include/modeMenu.h"


#define TPCharged     (!digitalRead(TP_CHARGED_PIN) && digitalRead(TP_CHARGING_PIN))
#define TPCharging    (!digitalRead(TP_CHARGING_PIN) && digitalRead(TP_CHARGED_PIN))

static uint32_t mAh, lastDischarge_ms;
static bool discharged, charged;


void modeSetup() {
  analogReference(INTERNAL);

  digitalWrite(RECOVERY_DIS_PIN, HIGH);   // Prevent glitch when setting it as output

  pinMode(TP_CHARGED_PIN, INPUT_PULLUP);
  pinMode(TP_CHARGING_PIN, INPUT_PULLUP);
  pinMode(RECOVERY_DIS_PIN, OUTPUT);
  pinMode(DISCHARGE_EN_PIN, OUTPUT);
  pinMode(CHARGE_EN_PIN, OUTPUT);

  DisableDischargeCurrent();
  digitalWrite(CHARGE_EN_PIN, LOW);
}

/**
 * Setup mAh measuring
 */
void mAhMeasuringSetup() {
  discharged = false;
  lastDischarge_ms = millis();
  mAh = 0;
}

#if HAS_RECOVERY
/**
 * Recovery
 */
void recovery(MenuAction action) {
  digitalWrite(CHARGE_EN_PIN, LOW);
  DisableDischargeCurrent();

  switch (action) {
    case MenuAction::pageEnter:
      printCaption("RECOVERY");
      printDischargeInfo(100);
      [[fallthrough]]

    case MenuAction::execute:
    case MenuAction::executeNoCaptionInfo:
      digitalWrite(RECOVERY_DIS_PIN, volt >= 3); // stop charge at 3V

      // Update HMI
      printMeasures();
      printStatus("Charge");
      break;

    case MenuAction::pageExit:
      break;

    case MenuAction::menuExit:
      digitalWrite(RECOVERY_DIS_PIN, HIGH);
      break;
  }
}
#endif

/**
 * Discharge battery
 */
void discharge(MenuAction action) {
  digitalWrite(CHARGE_EN_PIN, LOW);
  digitalWrite(RECOVERY_DIS_PIN, HIGH);

  switch(action) {
    case MenuAction::pageEnter:
      printCaption("DISCHARGE");
      mAhMeasuringSetup();
      [[fallthrough]]

    case MenuAction::execute:
      printCaptionInfo(-1, discharge_mA);
      [[fallthrough]]

    case MenuAction::executeNoCaptionInfo:
      if (!batteryPresent) {
        if (discharged) {
          oled.set2X();
          oled.setCursor(OLED_VALUE1_X, OLED_ROW1);
          oled.clearToEOL();
          oled.setCursor(OLED_VALUE1_X, OLED_ROW2);
          oled.clearToEOL();
          discharged = false;
        }
        mAhMeasuringSetup();
        printStatus("No battery");
      } else {
      if (!discharged) {
        // Discharge to minimum allowed voltage (with some margin)
        if (volt > 2.6)
          EnableDischargeCurrent();
        else {
          DisableDischargeCurrent();
          discharged = true;
        }

        const unsigned long ms = millis();
        if ((ms - lastDischarge_ms) >= 1000) {
          lastDischarge_ms = ms;
          mAh += abs(mA);
        }
      }

      // Update HMI
      printMeasures();
      oled.set2X();
      oled.setCursor(OLED_VALUE2_X, OLED_ROW2);
      printAligned(mAh / 3600, 4);
      oled.set1X();
      oled.setRow(OLED_ROW2 + 1);
      oled.print("mAh");
      if (discharged)
        printStatus("No load");
      else
        printStatus("Load on");
      }
      break;

    case MenuAction::pageExit:
      break;

    case MenuAction::menuExit:
      DisableDischargeCurrent();
      break;
  }
}

/**
 * Charge battery
 */
void charge(MenuAction action) {
  digitalWrite(RECOVERY_DIS_PIN, HIGH);
  DisableDischargeCurrent();

  switch (action) {
    case MenuAction::pageEnter:
      printCaption("CHARGE");
      [[fallthrough]]

    case MenuAction::execute:
      printCaptionInfo(charge_mA);
      [[fallthrough]]

    case MenuAction::executeNoCaptionInfo:
      charged = TPCharged && volt > 4.1;
      digitalWrite(CHARGE_EN_PIN, !charged);

      // Update HMI
      printMeasures();
      if (charged)
        printStatus("Charged (relay off)");
      else if (TPCharging)
        printStatus("Charging (relay on)");
      else
        printStatus("Ready");
      break;

    case MenuAction::pageExit:
      break;

    case MenuAction::menuExit:
      digitalWrite(CHARGE_EN_PIN, LOW);
      break;
  }
}

/**
 * Battery capacity test
 */
void capacityTest(MenuAction action) {
  switch (action) {
    case MenuAction::pageEnter:
      printCaption("CAPACITY");
      charged = discharged = false;
      [[fallthrough]]

    case MenuAction::execute:
      printCaptionInfo(charge_mA, discharge_mA);
      action = MenuAction::executeNoCaptionInfo;
      [[fallthrough]]

    case MenuAction::executeNoCaptionInfo:
    case MenuAction::menuExit:
      if (TPCharging || !charged) {
        charge(action);
        if (charged)
          mAhMeasuringSetup();    // Let 'Discharge()' correctly manage mAh
      }
      if (charged)
        discharge(action);
      break;

    case MenuAction::pageExit:
      break;
  }
}

/**
 * Check battery health
 */
void checkHealth(MenuAction action) {
  static uint32_t ms;
  static float openV;
  static uint8_t phase;

  digitalWrite(RECOVERY_DIS_PIN, HIGH);
  digitalWrite(CHARGE_EN_PIN, LOW);

  switch (action) {
    case MenuAction::pageEnter:
      printCaption("CHECK HEALTH");
      phase = 0;
      [[fallthrough]]

    case MenuAction::execute:
      printCaptionInfo(-1, discharge_mA);
      [[fallthrough]]

    case MenuAction::executeNoCaptionInfo:
      switch (phase) {
        case 0:
          printStatus("Testing...");
          ms = millis();
          phase = 10;
          break;

        case 10:
          if (millis() - ms >= 500) {
            ms += 500;
            printMeasures();
            openV = volt; // Save V open for mOhm calculation
            EnableDischargeCurrent();
            phase = 20;
          }
          break;

        case 20:
          if ((millis() - ms) >= 500) {
            readMeasures();
            const bool shortCircuit = (volt < 1);
            // Stop test
            DisableDischargeCurrent();

            // Update HMI
            int16_t absmA = abs(mA);

            printMeasures();
            if (shortCircuit && absmA < 10) {
              oled.clearField(OLED_VALUE2_X, OLED_ROW2, 4);
              printStatus("Short");
            } else {
              oled.set2X();
              if (absmA != 0 && openV >= volt) {
                const uint16_t mOhm = uint16_t((openV - volt) * 1000000 / absmA);
                oled.setCursor(OLED_VALUE2_X, OLED_ROW2);
                printAligned(mOhm, 4);
                oled.set1X();
                oled.setRow(OLED_ROW2 + 1);
                oled.print("m\xEA"); // mΩ
              }
              const bool isWeak = (volt <= 2.6 || absmA <= 150);
              printStatus(isWeak ? "Weak": "Good");
              phase = 0;
            }
          }
          break;
      }
      break;

    case MenuAction::pageExit:
      break;

    case MenuAction::menuExit:
      DisableDischargeCurrent();
      break;
  }
}

/**
 * Mode menu level handling
 */ 
void modeHandler(MenuAction action) {
  // Read charge current
  const int16_t iSel = analogRead(TP_ISEL_PIN);
  if (iSel > (int)(0.75 / 1.1 * 1024))
    charge_mA =(int32_t)1000 * 1200 / 1200;
  else if (iSel > (int)(0.35 / 1.1 * 1024))
    charge_mA = (int32_t)1000 * 1200 / 2400;
  else
    charge_mA = (int32_t)1000 * 1200 / 5600;

  // Read discharge current
  discharge_mA += (20 * encoderIncrement);
  if (discharge_mA < MIN_DISCHARGE_mA)
    discharge_mA = MIN_DISCHARGE_mA;
  else if (discharge_mA > MAX_DISCHARGE_mA)
    discharge_mA = MAX_DISCHARGE_mA;

  // Handle current page
  switch ((Mode)menu[menuLevel].curPage) {
    case Mode::health:
      checkHealth(action);
      break;

    case Mode::capacity:
      capacityTest(action);
      break;

    case Mode::charge:
      charge(action);
      break;

    #if HAS_RECOVERY
    case Mode::recovery:
      recovery(action);
      break;
    #endif

    case Mode::discharge:
      discharge(action);
      break;
  }
}
