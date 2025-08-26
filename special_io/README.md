# Special I/O Instruments

This folder contains specialized instrument drivers for programming interfaces, log parsers, and system utilities.

## Programming & Debug Interfaces
- **jlink_driver.py** - Segger J-Link JTAG/SWD Programmer (JFlash subprocess)
- **flashpro_driver.py** - Microsemi FlashPro FPGA Programmer (FlashPro-ARM subprocess)

## Data Acquisition & Analysis
- **frequency_counter.py** - Frequency measurement instruments
- **analog_in.py** - Generic analog input interfaces
- **ni_analog_in.py** - National Instruments analog input modules

## Log Processing & Test Integration
- **iqfact.py** - LitePoint IQfact+ Log Parser (File-based log analysis)

## System Utilities
- **nircmd_win_driver.py** - Windows NirCmd System Utility Driver (Windows-specific commands)

## Communication Characteristics
- **File I/O**: Log parsers read/write test result files
- **Subprocess Execution**: Programming tools execute external binaries  
- **System Commands**: OS-level utility integration
- **Proprietary APIs**: Specialized hardware interfaces

## Common Usage Pattern
```python
# Programming tool example:
subprocess.Popen(f'{tool_path} {script_file}', shell=True)

# Log parser example:
with open(log_file, 'r') as f:
    parse_test_results(f.readlines())
```