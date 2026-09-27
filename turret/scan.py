import serial
import time
import math
import matplotlib.pyplot as plt

PORT = "COM4"
MAX_RANGE = 300

ser = serial.Serial(PORT, 115200, timeout=1)
print("waiting for arduino...")
time.sleep(5)

xs, ys, zs, ds = [], [], [], []

print("scanning, this takes a few minutes")
while True:
    line = ser.readline().decode(errors="ignore").strip()
    if not line:
        continue
    if line == "scan done":
        break
    if "," not in line:
        print(line)
        continue

    try:
        p, t, d = [int(v) for v in line.split(",")]
    except ValueError:
        continue

    if d < 2 or d > MAX_RANGE:
        continue

    az = math.radians(p - 115)
    el = math.radians(t - 37)

    x = d * math.cos(el) * math.sin(az)
    y = d * math.cos(el) * math.cos(az)
    z = d * math.sin(el)

    xs.append(x); ys.append(y); zs.append(z); ds.append(d)
    print(f"{len(xs)} points", end="\r")

ser.close()
print(f"\ncollected {len(xs)} points")

fig = plt.figure(figsize=(9, 7))
ax = fig.add_subplot(projection="3d")
sc = ax.scatter(xs, ys, zs, c=ds, cmap="viridis", s=12)
ax.set_xlabel("x (cm)")
ax.set_ylabel("y (cm)")
ax.set_zlabel("z (cm)")
fig.colorbar(sc, label="distance (cm)")
plt.show()