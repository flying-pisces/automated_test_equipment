# USB I/O Instruments

This folder contains instrument drivers that communicate directly over USB interfaces (non-VISA).

## Instruments

### Test Equipment Control
- **mcci_usb.py** - MCCI USB Connection Exerciser (Subprocess calls to MCCI binary)

### Data Acquisition  
- **ni_usb_dio_driver.py** - National Instruments USB-6501 Digital I/O (Subprocess to NI utilities)

### Imaging Systems
- **pg-camera.py** - Point Grey/FLIR Camera using Spinnaker SDK (PySpin library)

## Communication Characteristics
- **Direct USB**: Uses manufacturer-specific drivers and libraries
- **Subprocess Integration**: Many drivers execute external binaries/utilities
- **SDK Libraries**: PySpin for cameras, proprietary libraries for specialized equipment

## Common Usage Pattern
```python
# Camera SDK example:
import PySpin
system = PySpin.System.GetInstance()

# Binary execution example:  
subprocess.Popen(command, stdout=subprocess.PIPE)
```