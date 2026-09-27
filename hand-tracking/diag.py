import cv2
import math
import mediapipe as mp

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils
H = mp_hands.HandLandmark

cap = cv2.VideoCapture(0)

with mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7) as hands:
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
            mid   = lm.landmark[H.MIDDLE_FINGER_MCP]
            idx   = lm.landmark[H.INDEX_FINGER_MCP]
            pinky = lm.landmark[H.PINKY_MCP]

            roll = math.degrees(math.atan2(mid.y - wrist.y, mid.x - wrist.x))

            length = math.hypot(mid.x - wrist.x, mid.y - wrist.y)
            width  = math.hypot(pinky.x - idx.x, pinky.y - idx.y)
            ratio  = length

            text = f"roll {roll:6.1f}   ratio {ratio:5.2f}"
            print(text)
            cv2.putText(frame, text, (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

        cv2.imshow("diag", frame)
        if cv2.waitKey(1) == 27:
            break

cap.release()
cv2.destroyAllWindows()