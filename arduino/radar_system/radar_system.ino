/*
  RADAR SYSTEM - Arduino Mega 2560 (Manual arrow-key control version)
  ---------------------------------------------------------------------
  HC-SR04 ultrasonic sensor mounted on a 28BYJ-48 stepper (ULN2003 driver).
  - Sweep is MANUAL: send 'L' or 'R' over Serial (e.g. from a laptop
    keyboard script) to rotate the sensor left/right.
  - A toggle switch on D2 turns the whole system on/off.
  - LED lights up + LCD shows "Object Detected" when something is
    within DETECT_RANGE_CM.

  Libraries needed (Sketch > Include Library > Manage Libraries):
    - Stepper (built-in with Arduino IDE)
    - LiquidCrystal_I2C  (by Frank de Brabander, or "LiquidCrystal I2C")

  Wiring:
    HC-SR04   VCC -> 5V   Trig -> D9   Echo -> D8   GND -> GND
    ULN2003   IN1 -> D22  IN2 -> D23   IN3 -> D24   IN4 -> D25
    LED       D13 -> 220ohm resistor -> LED anode; LED cathode -> GND
    LCD I2C   SDA -> D20  SCL -> D21   VCC -> 5V    GND -> GND
    Switch    one leg -> D2   other leg -> GND (uses internal pull-up)
*/

#include <Stepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- USER SETTINGS ----------
const int DETECT_RANGE_CM   = 30;    // alert threshold
const int SWEEP_STEPS_TOTAL = 512;   // 28BYJ-48 steps/rev used by Stepper library
const int SWEEP_ARC_STEPS   = 128;   // soft limit each direction from center
const int STEP_INCREMENT    = 8;     // steps moved per arrow key press
const int STEPPER_RPM       = 10;    // stepper speed
const int LCD_I2C_ADDRESS   = 0x27;  // common default; try 0x3F if display shows nothing
const long BAUD_RATE        = 9600;

// ---------- HARDWARE SETUP ----------
#define TRIG_PIN   9
#define ECHO_PIN   8
#define LED_PIN    13
#define SWITCH_PIN 2

// 28BYJ-48 wiring order for Stepper library must be IN1,IN3,IN2,IN4
Stepper radarStepper(SWEEP_STEPS_TOTAL, 22, 24, 23, 25);

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 16, 2);

int position = 0;       // current step position relative to center
bool wasOff = false;    // tracks previous switch state, so we only redraw LCD on change

void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  radarStepper.setSpeed(STEPPER_RPM);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Radar Starting");
  delay(1000);
  lcd.clear();
}

void loop() {
  bool systemOn = (digitalRead(SWITCH_PIN) == LOW); // switch closed to GND = ON

  if (!systemOn) {
    digitalWrite(LED_PIN, LOW);
    if (!wasOff) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System OFF");
      wasOff = true;
    }
    delay(100);
    return; // skip sensing/motor while off
  }

  if (wasOff) {
    lcd.clear();
    wasOff = false;
  }

  // ---- read arrow-key commands from laptop over Serial ----
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'L' && position - STEP_INCREMENT >= -SWEEP_ARC_STEPS) {
      radarStepper.step(-STEP_INCREMENT);
      position -= STEP_INCREMENT;
    } else if (cmd == 'R' && position + STEP_INCREMENT <= SWEEP_ARC_STEPS) {
      radarStepper.step(STEP_INCREMENT);
      position += STEP_INCREMENT;
    }
  }

  long distance = readDistanceCM();
  updateDisplay(distance);
}

// Reads distance from HC-SR04 in centimeters
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (~5m max range)
  if (duration == 0) {
    return -1; // no echo received / out of range
  }
  return duration * 0.0343 / 2;
}

Serial.println(digitalRead(SWITCH_PIN));

void updateDisplay(long distanceCm) {
  lcd.setCursor(0, 0);

  if (distanceCm > 0 && distanceCm <= DETECT_RANGE_CM) {
    digitalWrite(LED_PIN, HIGH);
    lcd.print("Object Detected ");
    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(distanceCm);
    lcd.print("cm    ");
  } else {
    digitalWrite(LED_PIN, LOW);
    lcd.print("Pos: ");
    lcd.print(position);
    lcd.print("       ");
    lcd.setCursor(0, 1);
    if (distanceCm > 0) {
      lcd.print("Dist: ");
      lcd.print(distanceCm);
      lcd.print("cm    ");
    } else {
      lcd.print("Dist: ---       ");
    }
  }
}
