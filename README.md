# Lithium battery doctor

**PROOF OF CONCEPT PROJECT**

**:warning::boom: WARNING:LITHIUM BATTERY MAY EASILY EXPLODE OR TAKE FIRE, REPLICATE THIS PROJECT AT YOUR OWN RISK:boom::warning:**

Complete solution for Li Ion batteries

![alt text](https://github.com/GMagician/Li-Ion-Doctor/blob/master/hardware/Li%20Ion%20Doctor.jpg?raw=true)

Features:<br />
  Charge, discharge, amp hours count, test for short or open circuit, weak battery indicator, measure internal resistance.

- Charge:<br />
  TP4056 module (integrated), selectable current ~200/500/1000mA. TP4056 circuit is monitored by Arduino.
- Discharge:<br />
  current up to 2000mA (step 20mA), PWM controlled. Internal resistance is calculated from battery voltage drop under load.
- Capacity:<br />
  Battery gets fully charged, then discharged, mAh on selected current load is counted.
- Recovery:<br />
  Deep discharged battery charge with 100mA current.<br /><br />

This is a revisited project originally developped by Akos Boda in 2022 (https://www.thingiverse.com/thing:5229303).