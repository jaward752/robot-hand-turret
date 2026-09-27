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

            mcp = lm.landmark[H.INDEX_FINGER_MCP]
            tip = lm.landmark[H.INDEX_FINGER_TIP]

            dx = tip.x - mcp.x
            dy = tip.y - mcp.y
            angle = math.degrees(math.atan2(dy, dx))
            length = math.hypot(dx, dy)

            text = f"angle {angle:6.1f}   len {length:5.3f}"
            print(text)
            cv2.putText(frame, text, (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

        cv2.imshow("point", frame)
        if cv2.waitKey(1) == 27:
            break

cap.release()
cv2.destroyAllWindows()