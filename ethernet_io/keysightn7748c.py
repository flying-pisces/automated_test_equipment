import pyvisa

# VISA address of the N7748C
visa_address = 'TCPIP0::100.65.16.193::inst0::INSTR'

# Connect to the instrument
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
    inst.close()

except pyvisa.VisaIOError as e:
    print(f"VISA communication error: {e}")



