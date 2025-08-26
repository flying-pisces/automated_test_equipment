# Serial Port Instruments

This folder contains instrument drivers that communicate over RS-232/RS-485 serial interfaces.

## Instruments

### Motion Control
- **DS102SerialPort.py** - DS102 Motion Controller (COM port, 38400 baud)
- **linmotasf.py** - LinMot Linear Motor Controller (Serial, binary protocol)

### Power & Load Equipment
- **dcload.py** - BK Precision DC Electronic Load (Serial, SCPI-like commands)
- **oven.py** - Temperature Oven Controller (Serial, 9600 baud, CRC16 protocol)

### Switching & Relay Control  
- **kta_223.py** - KTA 223 Relay Controller (Serial interface)

## Communication Characteristics
- **Baud Rates**: 9600, 19200, 38400 baud depending on instrument
- **Protocols**: ASCII commands, binary protocols, some with CRC checksums
- **Interfaces**: COM ports (Windows), /dev/tty* (Linux/macOS)

## Common Usage Pattern
```python
import serial

# Typical serial configuration:
port = serial.Serial('COM3', 38400, timeout=1)
port.write(b'COMMAND\r\n')
response = port.read()
```