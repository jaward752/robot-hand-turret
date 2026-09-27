import cv2
import serial
import time
import mediapipe as mp

PORT = "COM4"

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils
H = mp_hands.HandLandmark

PAN_MIN, PAN_MAX = 30, 175
TILT_MIN, TILT_MAX = 0, 75

def remap(v, out_min, out_max):
    v = max(0.0, min(1.0, v))
    return int(out_min + v * (out_max - out_min))

ser = serial.Serial(PORT, 115200, timeout=0)
print("waiting for arduino to boot...")
time.sleep(5)
print("go")

cap = cv2.VideoCapture(0)

with mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.5) as hands:
    while True:
        ok, frame = cap.read()
        if not ok:
            break

        frame = cv2.flip(frame, 1)
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        result = hands.process(rgb)

        if result.multi_hand_landmarks:
            lm = result.multi_hand_landmarks[0]
            mp_draw.draw_landmarks(frame, lm, mp_hands.HAND_CONNECTIONS)

            wrist = lm.landmark[H.WRIST]
            pan = remap(wrist.x, PAN_MIN, PAN_MAX)
            tilt = remap(1.0 - wrist.y, TILT_MIN, TILT_MAX)

            ser.write(f"p{pan}\n".encode())
            ser.write(f"t{tilt}\n".encode())

            reply = ser.readline()
            if reply:
                print("arduino:", reply)

            text = f"pan {pan}  tilt {tilt}"
            print(text)
            cv2.putText(frame, text, (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

        cv2.imshow("control", frame)
        if cv2.waitKey(1) == 27:
            break

cap.release()
ser.close()
cv2.destroyAllWindows()