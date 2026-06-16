import serial
import time
from pathlib import Path

PORT = "COM10"
BAUD = 115200
OUTFILE = Path(r"C:\Documents\2026-Summer-Industrial-Automation-Projects\Project_B\data2.csv")

OUTFILE.parent.mkdir(parents=True, exist_ok=True)

try:
    with serial.Serial(PORT, BAUD, timeout=2) as ser, open(OUTFILE, "w", encoding="utf-8") as f:
        time.sleep(2)              # wait for ESP32 reset after serial connection
        ser.reset_input_buffer()   # discard startup junk


        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()

            if line:
                print(line)
                f.write(line + "\n")
                f.flush()

except KeyboardInterrupt:
    print("\nStopped.")

except serial.SerialException as e:
    print(f"Serial error: {e}")
    print("Check that Arduino Serial Monitor/Plotter is closed and COM10 is correct.")