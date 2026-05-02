import serial
import time
import matplotlib.pyplot as plt

PORT = '/dev/ttyACM0'
BAUD = 9600
MAX_ENTRIES = 50

# getting serial port
ser = serial.Serial(PORT, BAUD, timeout=1)
ser.reset_input_buffer()

# Deque the last MAX_ENTRIES entries
data_history = collections.deque([0]*MAX_ENTRIES, maxlen=MAX_ENTRIES)

# Initializing the plot
fig, ax = plt.subplots()
line, = ax.plot(data_history)
ax.setlim(0, 255) # byte range
ax.set_title("Master Realtime data stream")
ax.set_ylable("Byte Value")
ax.set_xlable("Entry Num")

print("Reading data from master...")

def update(frame):
    if ser.in_waiting > 0:
        raw = ser.read(1)
        val = ord(raw)

        data_history.append(val)
        line.set_ydata(data_history)

    return line

ani = FunctionAnimation(fig, update, interval=5, blit=True)

try:
    plt.show()
except KeyboardInterrupt:
    ser.close()