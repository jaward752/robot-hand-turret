# Robot Hand Turret

A hand-tracking controlled pan/tilt turret with an ultrasonic distance sensor,
used to steer the turret with hand gestures and build 3D radar-style scans of
a room.

## How the pieces fit together

```
webcam --> hand-tracking/ --> (wrist position) --> turret/ --> serial --> arduino/ --> servos + ultrasonic sensor
                                                        |
                                                        +--> live radar view / 3D point cloud
```

1. **`hand-tracking/`** — standalone computer-vision scripts. They use a
   webcam and MediaPipe to detect a hand and read out landmark positions
   (wrist location, roll, finger-pointing angle). Useful for testing hand
   detection on its own, with no Arduino attached.
   - `hand.py` — tracks the hand and prints the wrist (x, y) position.
   - `diag.py` — prints hand roll angle and a size ratio, for calibration.
   - `point.py` — prints the index-finger pointing angle and length.

2. **`turret/`** — combines hand tracking with serial communication to the
   Arduino. The wrist x/y position is remapped to pan/tilt servo angles and
   sent over serial as `p<angle>` / `t<angle>` commands.
   - `control.py` — sends pan/tilt commands from wrist position; prints the
     Arduino's replies. Good first script to confirm the servos respond.
   - `live.py` — same hand-tracking control, but also reads back
     `pan,tilt,distance` readings from the Arduino's ultrasonic sensor and
     draws a live top-down radar view.
   - `scan.py` — no hand tracking; drives a full automatic sweep on the
     Arduino side and reads back the `pan,tilt,distance` stream to build a
     3D point cloud (matplotlib) of whatever the sensor scanned.

3. **`arduino/`** — sketches that run on the Arduino itself, each in its own
   folder (required by the Arduino IDE, which expects the folder name to
   match the `.ino` file name).
   - `radar_system/radar_system.ino` — core pan/tilt + ultrasonic sensor
     sketch: moves the servos and reports distance readings over serial.
   - `turret/turret.ino` — turret motion sketch (pan/tilt control logic).
   - `live_radar/live_radar.ino` — Arduino-side counterpart to `turret/live.py`,
     streaming live `pan,tilt,distance` data for the on-screen radar view.

## Running it

1. Flash the Arduino with the appropriate sketch from `arduino/`:
   - Use `radar_system/radar_system.ino` (or `live_radar/live_radar.ino` for
     the live-view workflow) via the Arduino IDE.
2. Note which serial port the Arduino enumerates as (e.g. `COM4` on Windows)
   and update the `PORT` variable at the top of the relevant Python script if
   it differs.
3. Install the Python dependencies:
   ```bash
   pip install opencv-python mediapipe pyserial numpy matplotlib
   ```
4. Run a script depending on what you want to do:
   - Test hand tracking only, no hardware: `python hand-tracking/hand.py`
   - Drive the turret live with your hand: `python turret/control.py` or
     `python turret/live.py` (adds the live radar display)
   - Run a full automatic 3D scan: `python turret/scan.py`
   - Press `Esc` in the video window to quit the OpenCV-based scripts.
