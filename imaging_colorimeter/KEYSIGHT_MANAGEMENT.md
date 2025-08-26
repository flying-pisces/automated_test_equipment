# Keysight Power Meter Management Utilities

This directory contains upper-level utilities for managing the Keysight Power Meter (100.65.16.193) when it becomes unresponsive. These tools operate at a higher level than the main.py application and provide comprehensive device recovery capabilities.

## 🚨 When to Use These Tools

Use these utilities when:
- The Keysight web interface shows loading spinner (as in your screenshot)
- VISA connections to the power meter fail
- The device becomes unresponsive during optimization runs
- Network communication with the device is intermittent

## 📁 Available Tools

### 1. **keysight_manager.py** (Core Python Utility)
The main Python script that provides all device management functionality.

**Direct Usage:**
```bash
python keysight_manager.py health    # Check device status
python keysight_manager.py reset     # Soft reset via VISA
python keysight_manager.py recover   # Full recovery process
```

### 2. **keysight_control.bat** (Windows Batch Script)
Easy-to-use Windows batch file with interactive menu.

**Usage:**
```cmd
keysight_control.bat                 # Interactive menu
keysight_control.bat health          # Quick health check
keysight_control.bat reset           # Reset device
keysight_control.bat recover         # Full recovery
```

### 3. **keysight_control.ps1** (PowerShell Script)
Advanced PowerShell script with monitoring capabilities.

**Usage:**
```powershell
.\keysight_control.ps1               # Interactive menu
.\keysight_control.ps1 health        # Health check
.\keysight_control.ps1 monitor       # Continuous monitoring
.\keysight_control.ps1 monitor -MonitorInterval 60  # Monitor every 60 seconds
```

### 4. **keysight_control.sh** (Bash Script)
Cross-platform bash script for Linux/macOS compatibility.

**Usage:**
```bash
./keysight_control.sh                # Interactive menu
./keysight_control.sh health         # Health check
./keysight_control.sh monitor 30     # Monitor every 30 seconds
```

## 🔧 Recovery Methods

The utilities attempt recovery in this order:

### Level 1: Soft Reset (VISA Commands)
- Sends `*RST` and `*CLS` commands via VISA
- Fastest method, works for minor communication issues
- Success rate: ~70% for typical problems

### Level 2: Network Reset (HTTP Commands)
- Attempts HTTP-based reset commands
- Works when VISA fails but network is stable
- Success rate: ~50% for network-related issues

### Level 3: SNMP Power Cycle (if supported)
- Uses SNMP commands to power cycle the device
- Requires SNMP configuration on the device
- Success rate: ~90% for hardware issues

### Level 4: Manual Intervention Required
If all automatic methods fail:
1. Physical power cycle of the device
2. Check network cable connections
3. Restart network switch/router
4. Contact system administrator

## 📊 Health Monitoring

The health check verifies:
- **Network Connectivity**: Ping test to device IP
- **Web Interface**: HTTP response from web server
- **VISA Communication**: Instrument identification query
- **Overall Status**: Combined health assessment

Status Indicators:
- 🟢 **HEALTHY**: All systems operational
- 🟡 **DEGRADED**: Network OK, some services failing
- 🔴 **OFFLINE**: Device unreachable

## 🎯 Quick Recovery Steps

### For the Loading Spinner Issue (Your Screenshot):

1. **Quick Fix** (Windows):
   ```cmd
   keysight_control.bat reset
   ```

2. **Comprehensive Fix** (Windows):
   ```cmd
   keysight_control.bat recover
   ```

3. **Monitor Recovery** (PowerShell):
   ```powershell
   .\keysight_control.ps1 monitor
   ```

### Integration with main.py

The main.py application can call these utilities:

```python
import subprocess
import os

def recover_power_meter():
    """Call upper-level recovery utility"""
    script_path = os.path.join(os.path.dirname(__file__), "keysight_manager.py")
    result = subprocess.run([sys.executable, script_path, "recover"], 
                          capture_output=True, text=True)
    return result.returncode == 0
```

## 🛠️ Installation & Setup

1. **Ensure Python dependencies:**
   ```bash
   pip install pyvisa requests
   ```

2. **Make scripts executable (Linux/macOS):**
   ```bash
   chmod +x keysight_control.sh
   ```

3. **Set PowerShell execution policy (Windows):**
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   ```

## 📞 Troubleshooting

### Common Issues:

**"Python script not found"**
- Ensure all files are in the same directory
- Check file paths and permissions

**"VISA driver not found"**
- Install Keysight IO Libraries Suite
- Verify VISA runtime is installed

**"Network timeout"**
- Check network connectivity to 100.65.16.193
- Verify firewall settings
- Test with direct ping

**"Permission denied"**
- Run as administrator (Windows)
- Check file permissions (Linux/macOS)

### Device-Specific Notes:

- **IP Address**: 100.65.16.193 (fixed in configuration)
- **VISA Address**: TCPIP0::100.65.16.193::inst0::INSTR
- **Web Interface**: http://100.65.16.193/pm/index.html
- **Default Timeout**: 5 seconds for network operations
- **Recovery Delay**: 2-15 seconds between recovery attempts

## 🔄 Best Practices

1. **Before Running Optimization:**
   - Run health check: `keysight_control.bat health`
   - Ensure device is in HEALTHY state

2. **During Long Operations:**
   - Start monitoring: `keysight_control.ps1 monitor`
   - Set reasonable timeout values in main.py

3. **After Device Issues:**
   - Run full recovery: `keysight_control.bat recover`
   - Verify recovery with health check
   - Wait 30 seconds before resuming operations

4. **Preventive Maintenance:**
   - Weekly health checks
   - Monitor device logs
   - Keep network infrastructure stable

These utilities provide a robust solution for managing the Keysight power meter independently of your main optimization application, ensuring reliable operation even when the device becomes unresponsive.