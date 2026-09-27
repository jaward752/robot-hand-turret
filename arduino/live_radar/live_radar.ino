#include <Servo.h>

const int PAN_PIN = 9, TILT_PIN = 10;
const int TRIG = 7, ECHO = 6;
const int PAN_CENTER = 115, PAN_MIN = 30, PAN_MAX = 175;
const int TILT_CENTER = 60, TILT_MIN = 0, TILT_MAX = 75;
const int MAX_STEP = 4;

Servo pan, tilt;
int p, t, pTarget, tTarget;
char buf[16];
byte n = 0;
unsigned long lastPing = 0;

int stepToward(int cur, int tgt) {
  int d = tgt - cur;
  if (d >  MAX_STEP) d =  MAX_STEP;
  if (d < -MAX_STEP) d = -MAX_STEP;
  return cur + d;
}

long readDistanceCm() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 20000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2;
}

void handle(char *s) {
  char axis = s[0];
  if (axis != 'p' && axis != 't') return;
  int v = atoi(s + 1);
  if (axis == 'p') pTarget = constrain(v, PAN_MIN, PAN_MAX);
  else             tTarget = constrain(v, TILT_MIN, TILT_MAX);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  p = pTarget = PAN_CENTER;
  t = tTarget = TILT_CENTER;
  pan.attach(PAN_PIN);  pan.write(p);
  delay(8);
  tilt.attach(TILT_PIN); tilt.write(t);
  Serial.println("ready");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (n > 0) { buf[n] = 0; handle(buf); n = 0; }
    } else if (n < 15) {
      buf[n++] = c;
    }
  }

  p = stepToward(p, pTarget);
  t = stepToward(t, tTarget);
  pan.write(p);
  delay(8);
  tilt.write(t);
  delay(12);

  if (millis() - lastPing > 100) {
    lastPing = millis();
    long d = readDistanceCm();
    if (d > 0) {
      Serial.print(p); Serial.print(",");
      Serial.print(t); Serial.print(",");
      Serial.println(d);
    }
  }
}