import serial
import time

# === Configuration ===
COM_PORT = 'COM3'   # Replace with your actual COM port
BAUD_RATE = 38400    # Confirm the baud rate from the manual
TIMEOUT = 1         # seconds

# === Commands (examples, might vary by controller model) ===
# These are ASCII commands; adapt if using binary protocol

CMD_HOME = b'H:1\r\n'           # Homing command for axis 1
CMD_MOVE_ABS = b'X:1,1000\r\n'  # Move axis 1 to absolute position 1000
CMD_GET_POS = b'Q:1\r\n'        # Query current position of axis 1
CMD_STOP = b'S:1\r\n'           # Stop motion on axis 1

# === Connect to serial port ===
try:
    DS102 = serial.Serial(COM_PORT, BAUD_RATE, timeout=TIMEOUT)
    time.sleep(2)  # Wait for connection to stabilize
    print(f"Connected to {COM_PORT} at {BAUD_RATE} baud.")
except serial.SerialException as e:
    print(f"Failed to connect: {e}")
    exit(1)

def send_command(cmd):
    """Send command and return response"""
    print(f"Sending: {cmd.decode().strip()}")
    DS102.write(cmd)
    time.sleep(0.1)
    response = DS102.read_all().decode().strip()
    print(f"Response: {response}")
    return response

# === Example Usage ===
try:
    send_command(CMD_HOME)
    time.sleep(5)  # Wait for homing to complete (adjust as needed)

    send_command(CMD_MOVE_ABS)
    time.sleep(2)

    pos = send_command(CMD_GET_POS)
    print(f"Stage Position: {pos}")

except Exception as e:
    print(f"Error during serial communication: {e}")
finally:
    DS102.close()
    print("Serial connection closed.")

DS102.write_termination = '\r'
DS102.read_termination = '\r'