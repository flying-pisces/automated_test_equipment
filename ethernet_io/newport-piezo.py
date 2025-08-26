import serial
import time
import pyvisa

# VISA address of the N7748C
visa_address = 'TCPIP0::100.65.16.193::inst0::INSTR'

# ===== Piezo configuration =====
PIEZO_COM_PORT = 'COM6'      # Update to your actual COM port
PIEZO_BAUD_RATE = 19200      # Default for NPC3SG
PIEZO_TIMEOUT = 1            # Serial timeout in seconds

# ===== DS102 configuration =====
DS102_COM_PORT = 'COM3'      # Update to your actual COM port
DS102_BAUD_RATE = 38400      # Default for NPC3SG
DS102_TIMEOUT = 1            # Serial timeout in seconds
# Connect to the Keysight
rm = pyvisa.ResourceManager()
try:
    inst = rm.open_resource(visa_address)
    inst.timeout = 5000  # Set timeout in ms

    # Identify the instrument
    idn = inst.query("*IDN?")
    print(f"Connected to: {idn.strip()}")

    # Configure channel 1 if needed (e.g., wavelength, range)
    inst.write("SENS:CHAN1:POW:WAV 1550NM")  # Set wavelength to 1550nm
    inst.write("SENS:CHAN1:POW:RANG:AUTO 1")  # Enable auto-ranging

    # Get power reading from channel 1
    power = inst.query("READ:CHAN1:POW?")
    print(f"Channel 1 Power: {float(power.strip()):.4f} W")
 #   inst.close()

    piezo = serial.Serial(PIEZO_COM_PORT, PIEZO_BAUD_RATE, timeout=PIEZO_TIMEOUT)
    time.sleep(1)  # Wait for connection to stabilize
    print(f"Piezo Connected to {PIEZO_COM_PORT}")
    DS102 = serial.Serial(DS102_COM_PORT, DS102_BAUD_RATE, timeout=DS102_TIMEOUT)
    time.sleep(1)  # Wait for connection to stabilize
    print(f"Piezo Connected to {DS102_COM_PORT}")

except serial.SerialException or pyvisa.VisaIOError as e:
    print(f"Piezo connection error: {e}")
    print(f"VISA communication error: {e}")
    exit(1)

# ===== piezo Command Functions =====
def send_cmd(cmd):
    full_cmd = cmd + '\r\n'
    print(f"Sending: {cmd}")
    piezo.write(full_cmd.encode())
    time.sleep(0.05)
    response = piezo.read_all().decode().strip()
    print(f"Response: {response}")
    return response

def move_absolute(channel: int, position: float):
    # Position in microns (0–100 typically)
    return send_cmd(f"{channel}PA{position:.2f}")

def move_relative(channel: int, delta: float):
    return send_cmd(f"{channel}PR{delta:.2f}")

def get_position(channel: int):
    return send_cmd(f"{channel}TP")

def stop_motion(channel: int):
    return send_cmd(f"{channel}ST")

# ===== Example Usage =====
try:
    send_cmd("1MO")              # Enable channel 1
    move_absolute(1, 50.0)       # Move to 50 microns
    time.sleep(1)
    get_position(1)
    move_relative(1, -10.0)      # Move back 10 microns
    time.sleep(1)
    get_position(1)
    stop_motion(1)

except Exception as e:
    print(f"Error: {e}")

finally:
    piezo.close()
    print("Piezo port closed.")
    inst.close()
    print("Keysight port closed.")
    DS102.close()
    print("DS102 port closed.")