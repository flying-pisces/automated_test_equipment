# Ethernet I/O Instruments

This folder contains instrument drivers that communicate over Ethernet using VISA/SCPI protocols.

## Instruments

### Test & Measurement Equipment
- **dmm_agilent_3446x.py** - Agilent/Keysight 34461A Digital Multimeter (TCPIP/USB VISA)
- **power_supply_agilent_e36xx.py** - Agilent E364x Power Supply series (VISA)  
- **mux_agilent_3497x.py** - Agilent 34972/34970 Multiplexer/Switch Matrix (VISA)
- **usb_power_sensor_keysight_u2001a.py** - Keysight U2001A Power Sensor (USB VISA)

### Optical Equipment  
- **keysightn7748c.py** - Keysight N7748C Power Meter (TCPIP VISA)
- **newport-piezo.py** - Newport Piezo Controller with Keysight integration (TCPIP + Serial)

## Communication Protocol
All instruments use VISA (Virtual Instrument Software Architecture) over:
- TCP/IP Ethernet connections (TCPIP::address::INSTR)
- USB connections using VISA drivers (USB::VID::PID::SERIAL::INSTR)

## Common Usage Pattern
```python
from visa_instrument import VisaInstrument

# Typical VISA resource strings:
# "TCPIP::192.168.1.100::INSTR"  
# "USB0::0x0957::0x1A07::MY53204664::INSTR"
```