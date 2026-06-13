import serial
import time

PORT = "COM9"

BAUD = 115200
OUTFILE = "C:\Documents\Industrial Automation\Arduino IDE\Project_A\data.csv"

ser = serial.Serial(PORT, BAUD, timeout=2)
time.sleep(2)
f = open(OUTFILE, "w")
print("Logging to", OUTFILE, "- press Ctrl+C to stop")

try:
    while True:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if line:
            print(line)
            f.write(line+"\n")
            f.flush()
except KeyboardInterrupt:
    print("\nStopped.")
    f.close()
    ser.close()