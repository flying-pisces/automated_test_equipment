# Automated Test Equipment Controller

A comprehensive Python framework for controlling laboratory instruments with unified interfaces and standardized GUI controls.

![Python](https://img.shields.io/badge/python-3.7+-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Status](https://img.shields.io/badge/status-active-brightgreen.svg)

## 🚀 Features

- **Universal Equipment Interface** - Standardized API for all instrument types
- **I/O-Based Organization** - Equipment categorized by communication interface
- **Unified GUI System** - Consistent control interface across all equipment
- **Multi-Equipment Coordination** - Manage multiple instruments simultaneously
- **Configuration Management** - JSON-based equipment configuration and data export
- **Real-Time Monitoring** - Live measurement display and status tracking
- **Extensible Architecture** - Easy to add new equipment types

## 📋 Supported Equipment

### 🌐 Ethernet/VISA Instruments
- **Agilent/Keysight Digital Multimeters** (34461A, 34460A, 34410A, 34411A)
- **Agilent/Keysight Power Supplies** (E3640A, E3646A, E3648A series)
- **Agilent/Keysight Switch Matrix** (34972A, 34970A)
- **Keysight Power Meters** (N7748C)
- **Newport Piezo Controllers**

### 🔌 Serial Port Instruments
- **KTA Relay Controllers** (KTA-223)
- **BK Precision DC Electronic Loads**
- **Temperature Oven Controllers**
- **LinMot Linear Motor Controllers**
- **DS102 Motion Controllers**

### 🔗 USB Instruments
- **FLIR/Point Grey Cameras** (PySpin SDK)
- **National Instruments USB-6501 Digital I/O**
- **MCCI USB Connection Exercisers**

### ⚡ Special I/O
- **Segger J-Link Programmers**
- **Microsemi FlashPro FPGA Programmers**
- **LitePoint Test Equipment** (IQfact+)
- **System Utilities and Analyzers**

## 🛠️ Installation

### Prerequisites

1. **Python 3.7+** with tkinter support
2. **NI-VISA Runtime** (for Ethernet/USB instruments)
3. **Equipment-specific drivers** (see [Dependencies](#dependencies))

### Quick Install

```bash
# Clone the repository
git clone https://github.com/yourusername/automated_test_equipment.git
cd automated_test_equipment

# Install Python dependencies
pip install -r requirements.txt

# Run the main controller
python equipment_controller.py
```

### Platform-Specific Setup

#### Windows
```bash
# Install NI-VISA Runtime
# Download from: https://www.ni.com/en-us/support/downloads/drivers/download.ni-visa.html

# Install PySpin SDK (for cameras)
# Download from: https://www.flir.com/products/spinnaker-sdk/

pip install -r requirements.txt
```

#### macOS
```bash
# Install NI-VISA Runtime (32-bit mode required)
export VERSIONER_PYTHON_PREFER_32_BIT=yes

# Install dependencies
pip install -r requirements.txt
```

#### Linux
```bash
# Install system dependencies
sudo apt-get install python3-tk python3-serial

# Install VISA libraries
sudo apt-get install libvisa0 visa-dev

pip install -r requirements.txt
```

## 🚀 Quick Start

### Method 1: Main Controller GUI

Launch the main equipment controller application:

```bash
python equipment_controller.py
```

1. Click "Add Equipment" to configure your instruments
2. Select equipment type and enter connection parameters
3. Click "Connect All" to establish connections
4. Click "Open Control Panel" to control individual equipment

### Method 2: Individual Equipment Control

```python
from ethernet_io.agilent_dmm_equipment import AgilentDMMEquipment
from universal_equipment_gui import create_equipment_gui

# Create equipment instance
dmm = AgilentDMMEquipment("TCPIP::192.168.1.100::INSTR", "Lab DMM")

# Option 1: Programmatic control
dmm.connect()
dmm.set_config({'measurement_function': 'voltage_dc', 'auto_range': True})
voltage = dmm.measure()
print(f"Measured: {voltage} V")
dmm.export_data("measurements.json")
dmm.disconnect()

# Option 2: GUI control
gui = create_equipment_gui(dmm)
gui.run()
```

### Method 3: Configuration File

Create `equipment_config.json`:

```json
{
  "DMM_Lab": {
    "type": "agilent_dmm",
    "config": {
      "name": "Laboratory DMM",
      "visa_address": "TCPIP::192.168.1.100::INSTR"
    }
  },
  "PowerSupply_Bench": {
    "type": "agilent_power_supply", 
    "config": {
      "name": "Bench Power Supply",
      "visa_address": "TCPIP::192.168.1.101::INSTR",
      "model": "e3646a"
    }
  }
}
```

Load configuration in main controller or run:
```bash
python example_usage.py
```

## 🏗️ Architecture

### Universal Equipment Interface

All equipment implements the `BaseEquipment` abstract class with standardized methods:

```python
class BaseEquipment(ABC):
    # Connection Management
    def connect() -> bool
    def disconnect() -> bool  
    def is_connected() -> bool
    
    # Configuration
    def get_config() -> Dict
    def set_config(config: Dict) -> bool
    def apply_config() -> bool
    def reset() -> bool
    
    # Measurement Operations  
    def start_measurement() -> bool
    def stop_measurement() -> bool
    def measure() -> Any
    
    # Data Management
    def export_data(filename: str, format: str) -> bool
    def get_measurement_data() -> List[Dict]
    def clear_data()
    
    # Status & Diagnostics
    def get_status() -> Dict
    def self_test() -> bool
    def get_identification() -> Dict
```

### GUI System

The universal GUI provides a consistent interface:

- **Left Panel**: Configuration attributes with auto-generated widgets
- **Right Panel**: Standard control methods and real-time status
- **Menu System**: File operations, equipment controls, help

### Project Structure

```
automated_test_equipment/
├── base_equipment.py              # Universal equipment interface
├── universal_equipment_gui.py     # Standardized GUI framework
├── equipment_controller.py        # Main multi-equipment application
├── example_usage.py              # Usage examples and tutorials
├── requirements.txt              # Python dependencies
├── equipment_config.json         # Example configuration
│
├── ethernet_io/                  # VISA/TCP instruments
│   ├── agilent_dmm_equipment.py
│   ├── agilent_power_supply_equipment.py
│   └── README.md
│
├── serial_port/                  # RS-232/485 instruments
│   ├── kta_relay_equipment.py
│   ├── dcload.py
│   ├── oven.py
│   └── README.md
│
├── usb_io/                       # Direct USB instruments  
│   ├── camera_equipment.py
│   ├── ni_usb_dio_driver.py
│   └── README.md
│
├── special_io/                   # Programming/system utilities
│   ├── jlink_driver.py
│   ├── flashpro_driver.py
│   └── README.md
│
└── [legacy components]/          # Original scripts and examples
    ├── Examples/
    ├── tests/
    └── docs/
```

## 📚 Documentation

### Adding New Equipment

1. **Create Equipment Class**:
```python
from base_equipment import BaseEquipment, IOType

class MyEquipment(BaseEquipment):
    def __init__(self, connection_params):
        super().__init__("My Equipment", IOType.ETHERNET, connection_params)
    
    # Implement all abstract methods
    def connect(self) -> bool:
        # Your connection logic
        pass
```

2. **Register in Factory**:
```python
# In equipment_controller.py
EQUIPMENT_TYPES = {
    'my_equipment': MyEquipment,
    # ... other types
}
```

3. **Test with Universal GUI**:
```python
equipment = MyEquipment(connection_params)
gui = create_equipment_gui(equipment)
gui.run()
```

### Configuration Format

Equipment configurations use JSON format:

```json
{
  "equipment_name": {
    "type": "equipment_type",  
    "config": {
      "name": "Display Name",
      "connection_param1": "value1",
      "connection_param2": "value2"
    }
  }
}
```

## 🧪 Testing

### Running Tests

```bash
# Legacy individual equipment tests
python -m tests.test_dmm_e34461 TCPIP::192.168.1.100::INSTR
python -m tests.test_ps_e3640 TCPIP::192.168.1.101::INSTR

# Test new equipment classes
python example_usage.py
```

### Equipment Validation

Each equipment class includes built-in self-test functionality:

```python
equipment = AgilentDMMEquipment("TCPIP::192.168.1.100::INSTR")
equipment.connect()
result = equipment.self_test()  # Returns True/False
```

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Setup

```bash
# Fork and clone the repository
git clone https://github.com/yourusername/automated_test_equipment.git
cd automated_test_equipment

# Create development environment
python -m venv venv
source venv/bin/activate  # Linux/macOS
# venv\Scripts\activate   # Windows

# Install development dependencies
pip install -r requirements-dev.txt

# Run tests
python -m pytest tests/
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **VISA Standard** - For standardized instrument communication
- **National Instruments** - NI-VISA runtime and drivers
- **Keysight Technologies** - SCPI command documentation
- **FLIR Systems** - Spinnaker SDK for camera integration
- **Python Community** - For excellent libraries and frameworks

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/automated_test_equipment/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/automated_test_equipment/discussions)
- **Documentation**: [Wiki](https://github.com/yourusername/automated_test_equipment/wiki)

## 🗺️ Roadmap

- [ ] **Web Interface** - Browser-based equipment control
- [ ] **Database Integration** - Measurement data storage and analysis
- [ ] **Automated Test Sequences** - Script-based test automation
- [ ] **Remote Access** - Network-based equipment sharing
- [ ] **Mobile App** - Smartphone equipment monitoring
- [ ] **Additional Equipment** - More instrument drivers

---

**Made with ❤️ for the test automation community**