#include <Servo.h>

const int PAN_PIN = 9, TILT_PIN = 10;
const int PAN_CENTER = 115, PAN_MIN = 30, PAN_MAX = 175;
const int TILT_CENTER = 60, TILT_MIN = 0, TILT_MAX = 75;
const int MAX_STEP = 4;
const unsigned long TIMEOUT_MS = 3000;

Servo pan, tilt;
int p, t, pTarget, tTarget;
unsigned long lastCmd = 0;

char buf[16];
byte n = 0;

int stepToward(int cur, int tgt) {
  int d = tgt - cur;
  if (d >  MAX_STEP) d =  MAX_STEP;
  if (d < -MAX_STEP) d = -MAX_STEP;
  return cur + d;
}

void handle(char *s) {
  char axis = s[0];
  if (axis != 'p' && axis != 't') return;

  int cur = (axis == 'p') ? pTarget : tTarget;
  int v;
  if (s[1] == '+' && s[2] == 0)      v = cur + 2;
  else if (s[1] == '-' && s[2] == 0) v = cur - 2;
  else                               v = atoi(s + 1);

  if (axis == 'p') pTarget = constrain(v, PAN_MIN, PAN_MAX);
  else             tTarget = constrain(v, TILT_MIN, TILT_MAX);

  lastCmd = millis();
  Serial.print("pan "); Serial.print(pTarget);
  Serial.print(" tilt "); Serial.println(tTarget);
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (n > 0) { buf[n] = 0; handle(buf); n = 0; }
    } else if (n < 15) {
      buf[n++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  p = pTarget = PAN_CENTER;
  t = tTarget = TILT_CENTER;
  pan.attach(PAN_PIN);  pan.write(p);
  delay(8);
  tilt.attach(TILT_PIN); tilt.write(t);
  Serial.println("ready");
}

void loop() {
  readSerial();

  if (millis() - lastCmd > TIMEOUT_MS) {
    pTarget = PAN_CENTER;
    tTarget = TILT_CENTER;
  }

  p = stepToward(p, pTarget);
  t = stepToward(t, tTarget);
  pan.write(p);
  delay(8);
  tilt.write(t);
  delay(12);
}