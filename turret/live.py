import cv2
import serial
import time
import math
import numpy as np
import mediapipe as mp

PORT = "COM4"
PAN_CENTER, TILT_LEVEL = 115, 37
PAN_MIN, PAN_MAX = 30, 175
TILT_MIN, TILT_MAX = 0, 75
MIN_CM, MAX_CM = 20, 250

SIZE = 500
CENTER = (SIZE // 2, SIZE - 40)
SCALE = (SIZE - 80) / MAX_CM

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils
H = mp_hands.HandLandmark

def remap(v, lo, hi):
    v = max(0.0, min(1.0, v))
    return int(lo + v * (hi - lo))

ser = serial.Serial(PORT, 115200, timeout=0)
print("waiting for arduino...")
time.sleep(5)
print("go")

points = []
pending = ""
cap = cv2.VideoCapture(0)

with mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.5) as hands:
    while True:
        ok, frame = cap.read()
        if not ok:
            break

        frame = cv2.flip(frame, 1)
        result = hands.process(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))

        if result.multi_hand_landmarks:
            lm = result.multi_hand_landmarks[0]
            mp_draw.draw_landmarks(frame, lm, mp_hands.HAND_CONNECTIONS)
            wrist = lm.landmark[H.WRIST]
            ser.write(f"p{remap(wrist.x, PAN_MIN, PAN_MAX)}\n".encode())
            ser.write(f"t{remap(1.0 - wrist.y, TILT_MIN, TILT_MAX)}\n".encode())

        waiting = ser.in_waiting
        if waiting:
            pending += ser.read(waiting).decode(errors="ignore")
            while "\n" in pending:
                line, pending = pending.split("\n", 1)
                line = line.strip()
                if "," not in line:
                    continue
                try:
                    p, t, d = [int(v) for v in line.split(",")]
                except ValueError:
                    continue
                if d < MIN_CM or d > MAX_CM:
                    continue
                points.append((p, t, d, time.time()))

        radar = np.zeros((SIZE, SIZE, 3), np.uint8)
        for r in range(50, MAX_CM + 1, 50):
            cv2.circle(radar, CENTER, int(r * SCALE), (0, 60, 0), 1)
            cv2.putText(radar, f"{r}", (CENTER[0] + 4, CENTER[1] - int(r * SCALE)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 90, 0), 1)
        for a in (PAN_MIN, PAN_CENTER, PAN_MAX):
            rad = math.radians(a - PAN_CENTER)
            cv2.line(radar, CENTER,
                     (int(CENTER[0] + MAX_CM * SCALE * math.sin(rad)),
                      int(CENTER[1] - MAX_CM * SCALE * math.cos(rad))),
                     (0, 60, 0), 1)

        now = time.time()
        for p, t, d, ts in points[-3000:]:
            age = now - ts
            if age > 20:
                continue
            fade = max(0.15, 1.0 - age / 20.0)
            rad = math.radians(p - PAN_CENTER)
            x = int(CENTER[0] + d * SCALE * math.sin(rad))
            y = int(CENTER[1] - d * SCALE * math.cos(rad))
            shade = int(60 + 195 * (1 - min(t, TILT_MAX) / TILT_MAX))
            cv2.circle(radar, (x, y), 2,
                       (int(shade * fade * 0.3), int(255 * fade), int(shade * fade)), -1)

        cv2.putText(radar, f"{len(points)} points", (10, 22),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

        cv2.imshow("control", frame)
        cv2.imshow("radar", radar)
        if cv2.waitKey(1) == 27:
            break

cap.release()
ser.close()
cv2.destroyAllWindows()