# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository contains the Orange RVS (Radiant Vision Systems) Instruments software package, a comprehensive testing and measurement system for display devices. The project is primarily a Windows-based .NET application ecosystem for visual display analysis and quality control.

## Architecture

This is a .NET Framework 4.8 application ecosystem consisting of:

- **TrueTest.exe** - Main Windows Forms application for display testing
- **MPK_API.dll** - Core measurement and pattern generation API
- **PMEngine.dll** - ProMetric measurement engine for photometric analysis
- **RadiantCamera.dll** - Camera hardware abstraction layer
- **Multiple analysis DLLs** - Specialized modules for different measurement types (ColorMura, ParticleDefects, LineMura, UniformityBase, etc.)

### Key Components

- **Measurement Engine**: PMEngine handles photometric measurements and calibration
- **Camera System**: G-SeriesCamera.dll and RadiantCamera.dll provide hardware interfaces
- **Analysis Modules**: Specialized DLLs for different types of visual defect detection:
  - ColorMura.dll - Color non-uniformity analysis
  - ParticleDefects.dll - Particle and contamination detection
  - LineMura.dll - Line-based defect analysis
  - UniformityBase.dll - Brightness and color uniformity
  - TrueMURA.dll - Advanced MURA (non-uniformity) analysis
- **Data Management**: Uses SQL Server Compact databases (.sdf files) and XML sequences (.seqx)
- **Pattern Generation**: TrueTestPatternGenerator.dll for test patterns

## Database Structure

- **Calibrations/**: RiCalibration.sdf for instrument calibration data
- **UserDefinitions/**: UserDefinitions_Master.sdf for user-defined settings
- **UserMaintenance/**: UserMaintenance_Master.sdf for maintenance records
- **PointsofInterest/**: RiPoints_Master.sdf for measurement point definitions

## Common Development Tasks

### Testing and API Usage

Run the Python test script to verify MPK API functionality:
```bash
python MPKAPItestScript.py
```

This script tests:
- Camera initialization
- Database operations
- Sequence execution
- Measurement export (PNG, CSV formats)

### COM Component Registration

For development environments, register COM components using:
```bash
RegWrapper.bat
```
Note: Must run as Administrator. This registers RadiantCommonCOM.dll and PMEngineCOM.dll in the Global Assembly Cache.

### Sequence Files

Test sequences are stored as .seqx XML files:
- `Data/DefaultSequence.seqx` - Default measurement sequence
- Pattern-specific sequences in root directory (e.g., `i16.seqx`)

## Configuration Files

The application uses multiple .config files for different components:
- `TrueTest.exe.config` - Main application configuration
- Individual DLL configurations for specialized modules
- Database connection strings and measurement parameters

## Data Flow

1. **Initialization**: Camera setup and calibration database loading
2. **Pattern Setup**: Test pattern selection and display configuration
3. **Measurement**: Camera capture with specified exposure and filter settings
4. **Analysis**: Specialized DLLs process captured images
5. **Results**: Data stored in measurement database and exported as needed

## File Locations

- **Firmware**: `/Firmware/` - FPGA bitstreams and device firmware
- **Camera Configs**: `/Camera/Camera Inifiles/` - Camera-specific initialization files
- **Patterns**: `/Patterns/` - Test pattern images
- **Resources**: `/Resources/` - Supporting utilities and libraries

## Python Integration

The system supports Python automation through the MPK_API.dll:
- IronPython integration via `clr.AddReference()`
- JSON-based communication for measurement data
- Automated sequence execution and export capabilities

## Multi-language Support

The application includes localized resources for:
- Japanese (ja/)
- Korean (ko/)
- Simplified Chinese (zh-CN/)
- Traditional Chinese (zh-TW/)

## Hardware Dependencies

- Radiant Vision Systems cameras (G-Series, etc.)
- CUDA-compatible GPU for accelerated processing (CUDA 8.0/10.1 support)
- Specialized measurement hardware (spectrometers, pattern generators)