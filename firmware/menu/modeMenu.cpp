#include "common.h"
#include "pins.h"
#include "encoder.h"
#include "include/modeMenu.h"


#define DisableDischargeCurrent() \
  do{ \
    analogWrite(DISCHARGE_PWM_PIN, 0); \
    digitalWrite(DISCHARGE_EN_PIN, LOW); \
    }while(0)
#define EnableDischargeCurrent() \
  do{ \
    /*analogWrite(DISCHARGE_PWM_PIN, 120 + digitalRead(SEL_500mA_PIN)*120);*/ \
    digitalWrite(DISCHARGE_EN_PIN, HIGH); \
    }while(0)

static uint32_t mAh, lastDischarge_ms;
static bool discharged, charged;
static int16_t charge_mA, discharge_mA = 200;


void modeSetup() {
  analogReference(INTERNAL);

  #if HAS_RECOVERY
  digitalWrite(RECOVERY_DIS_PIN, HIGH);   // Prevent glitch when setting it as output
  #endif

  pinMode(TP_CHARGED_PIN, INPUT_PULLUP);
  pinMode(TP_CHARGING_PIN, INPUT_PULLUP);
  #if HAS_RECOVERY
  pinMode(RECOVERY_DIS_PIN, OUTPUT);
  #endif
  pinMode(DISCHARGE_EN_PIN, OUTPUT);
  pinMode(CHARGE_EN_PIN, OUTPUT);

  DisableDischargeCurrent();
  digitalWrite(CHARGE_EN_PIN, LOW);
}

/**
 * Setup mAh measuring
 */
void mAhMeasuringSetup() {
  lastDischarge_ms = millis();
  mAh = 0;
}

#if HAS_RECOVERY
/**
 * Recovery
 */
void recovery(MenuAction action) {
  if (action == MenuAction::pageChange || action == MenuAction::menuExit)
    digitalWrite(RECOVERY_DIS_PIN, HIGH);
  else {
    digitalWrite(RECOVERY_DIS_PIN, volt >= 3); // stop charge at 3V

    // Update HMI
    if (action == MenuAction::pageEnter)
      printCaption("RECOVERY");
    printCaptionInfo(-1, 100);
    printMeasures();
    printStatus("Charge");
  }
}
#endif

/**
 * Discharge battery
 */
void discharge(MenuAction action) {
  if (action == MenuAction::pageChange || action == MenuAction::menuExit)
    DisableDischargeCurrent();
  else if (action == MenuAction::pageEnter) {
    printCaption("DISCHARGE");
    //if (!batteryPresent)
    discharged = false;
    mAhMeasuringSetup();
  } else {
    if (action != MenuAction::asa)
      printCaptionInfo(-1, discharge_mA);
    if (!batteryPresent)
      printStatus("No battery");
    else {
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
          mAh += mA;
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
  }
}

/**
 * Charge battery
 */
void charge(MenuAction action) {
  if (action == MenuAction::pageChange || action == MenuAction::menuExit)
    digitalWrite(CHARGE_EN_PIN, LOW);
  else {
    bool ready = digitalRead(TP_CHARGED_PIN) && !digitalRead(TP_CHARGING_PIN);
    charged = ready && volt > 4.1;
    digitalWrite(CHARGE_EN_PIN, !charged);

    // Update HMI
    if (action == MenuAction::pageEnter)
      printCaption("CHARGE");
    if (action != MenuAction::asa)
      printCaptionInfo(charge_mA, -1);

    printMeasures();
    const bool isCharging = digitalRead(TP_CHARGING_PIN) && !digitalRead(TP_CHARGED_PIN);
    if (!isCharging)
      printStatus("Ready");
    else if (charged)
      printStatus("Charged (relay off)");
    else
      printStatus("Charging (relay on)...");
  }
}

/**
 * Battery capacity test
 */
void capacityTest(MenuAction action) {
  if (action == MenuAction::pageEnter) {
    printCaption("CAPACITY");
    charged = discharged = false;
  }

  if (action != MenuAction::pageChange && action != MenuAction::menuExit)
    action = MenuAction::asa;
  printCaptionInfo(charge_mA, discharge_mA);

  if ((digitalRead(TP_CHARGING_PIN) && !digitalRead(TP_CHARGED_PIN)) || !charged) {
    charge(action);
    if (charged)
      mAhMeasuringSetup();    // Let 'Discharge()' correctly manage mAh
  }
  if (charged)
    discharge(action);
}

/**
 * Check battery health
 */
void checkHealth(MenuAction action) {
  static uint32_t ms;
  static float openV;
  static uint8_t phase;

  switch (action) {
    case MenuAction::pageChange:
    case MenuAction::menuExit:
      DisableDischargeCurrent();
      break;

    case MenuAction::pageEnter:
      printCaption("CHECK HEALTH");
      phase = 0;
      break;

    default:
      printCaptionInfo(-1, discharge_mA);

      switch (phase) {
        case 0:
          printStatus("Testing...");
          ms = millis();
          phase = 10;
          break;

        case 10:
          if ((millis() - ms) >= 500) {
            printMeasures();
            openV = volt; // Save V open for mOhm calculation
            EnableDischargeCurrent();
            phase = 20;
          }
          break;

        case 20:
          if ((millis() - ms) >= 1000) {
            readMeasures();
            const bool shortCircuit = (volt < 1);
            // Stop test
            DisableDischargeCurrent();

            // Update HMI
            printMeasures();
            if (shortCircuit && mA < 10) {
              oled.clearField(OLED_VALUE2_X, OLED_ROW2, 4);
              printStatus("Short");
            } else {
              oled.set2X();
              if (mA > 0) {
                const uint16_t mOhm = uint16_t((openV - volt) * 1000000 / mA);
                oled.setCursor(OLED_VALUE2_X, OLED_ROW2);
                printAligned(mOhm, 4);
                oled.set1X();
                oled.setRow(OLED_ROW2 + 1);
                oled.print("m\xEA"); // mΩ
              }
              const bool isWeak = (volt <= 2.6 || mA <= 150);
              printStatus(isWeak ? "Weak": "Good");
              phase = 0;
            }
          }
          break;
      }
    }
}

/**
 * Mode menu level handling
 */ 
void modeHandler(MenuAction action) {
  // Read charge current
  const int16_t iSel = analogRead(TP_ISEL_PIN);
  if (iSel > (int)(0.75 / 1.1 * 1024))
    charge_mA =(int32_t)1000 * 1200 / 5600;
  else if (iSel > (int)(0.35 / 1.1 * 1024))
    charge_mA = (int32_t)1000 * 1200 / 2400;
  else
    charge_mA = (int32_t)1000 * 1200 / 1200;

  // Read discharge current
  discharge_mA += encoderIncrement * 10;
  if (discharge_mA < 100)
    discharge_mA = 100;
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
