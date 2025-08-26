#!/usr/bin/env python3
"""
Keysight Power Meter Management Utility
Upper-level control for device reboot and health monitoring
Separate from main.py application
"""

import pyvisa
import requests
import time
import subprocess
import sys
import argparse
from datetime import datetime

# Configuration
POWER_METER_ADDRESS = "TCPIP0::100.65.16.193::inst0::INSTR"
POWER_METER_IP = "100.65.16.193"
WEB_INTERFACE_URL = f"http://{POWER_METER_IP}/pm/index.html"
PING_TIMEOUT = 5  # seconds

class KeysightManager:
    """Keysight Power Meter Management Class"""
    
    def __init__(self):
        self.ip = POWER_METER_IP
        self.visa_address = POWER_METER_ADDRESS
        self.web_url = WEB_INTERFACE_URL
        
    def log(self, message):
        """Log messages with timestamp"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] {message}")
    
    def ping_device(self):
        """Ping the device to check network connectivity"""
        try:
            result = subprocess.run(
                ['ping', '-n', '1', '-w', str(PING_TIMEOUT * 1000), self.ip],
                capture_output=True, text=True, timeout=10
            )
            return result.returncode == 0
        except subprocess.TimeoutExpired:
            return False
        except Exception as e:
            self.log(f"Ping failed: {e}")
            return False
    
    def check_web_interface(self):
        """Check if web interface is responding"""
        try:
            response = requests.get(self.web_url, timeout=5)
            return response.status_code == 200
        except requests.RequestException as e:
            self.log(f"Web interface check failed: {e}")
            return False
    
    def check_visa_connection(self):
        """Check VISA instrument connection"""
        try:
            rm = pyvisa.ResourceManager()
            inst = rm.open_resource(self.visa_address)
            inst.timeout = 2000
            
            # Try to get instrument ID
            idn = inst.query("*IDN?")
            inst.close()
            rm.close()
            
            self.log(f"VISA connection OK: {idn.strip()}")
            return True
            
        except Exception as e:
            self.log(f"VISA connection failed: {e}")
            return False
    
    def soft_reset_visa(self):
        """Attempt soft reset via VISA commands"""
        try:
            rm = pyvisa.ResourceManager()
            inst = rm.open_resource(self.visa_address)
            inst.timeout = 5000
            
            # Send reset commands
            inst.write("*RST")
            time.sleep(2)
            inst.write("*CLS")  # Clear status
            time.sleep(1)
            
            # Verify reset worked
            idn = inst.query("*IDN?")
            inst.close()
            rm.close()
            
            self.log(f"Soft reset successful: {idn.strip()}")
            return True
            
        except Exception as e:
            self.log(f"Soft reset failed: {e}")
            return False
    
    def hard_reset_network(self):
        """Attempt network-based reset (if supported by device)"""
        try:
            # Some Keysight devices support HTTP-based reset
            reset_url = f"http://{self.ip}/pm/api/system/reset"
            response = requests.post(reset_url, timeout=10)
            
            if response.status_code == 200:
                self.log("Network reset command sent successfully")
                return True
            else:
                self.log(f"Network reset failed: HTTP {response.status_code}")
                return False
                
        except requests.RequestException as e:
            self.log(f"Network reset not supported or failed: {e}")
            return False
    
    def power_cycle_via_snmp(self):
        """Attempt power cycle via SNMP (if supported)"""
        try:
            # This would require SNMP configuration on the device
            # Example command (would need actual SNMP OIDs for your device)
            cmd = [
                'snmpset', '-v2c', '-c', 'private', self.ip,
                '1.3.6.1.4.1.someOID', 'i', '1'  # Reset OID
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
            
            if result.returncode == 0:
                self.log("SNMP power cycle successful")
                return True
            else:
                self.log(f"SNMP power cycle failed: {result.stderr}")
                return False
                
        except Exception as e:
            self.log(f"SNMP power cycle not available: {e}")
            return False
    
    def check_device_health(self):
        """Comprehensive device health check"""
        self.log("=== Keysight Power Meter Health Check ===")
        
        health_status = {
            'network_ping': False,
            'web_interface': False,
            'visa_connection': False,
            'overall_status': 'UNKNOWN'
        }
        
        # Check network connectivity
        self.log("Checking network connectivity...")
        health_status['network_ping'] = self.ping_device()
        
        if health_status['network_ping']:
            self.log("✓ Network ping successful")
            
            # Check web interface
            self.log("Checking web interface...")
            health_status['web_interface'] = self.check_web_interface()
            
            if health_status['web_interface']:
                self.log("✓ Web interface responding")
            else:
                self.log("✗ Web interface not responding")
            
            # Check VISA connection
            self.log("Checking VISA connection...")
            health_status['visa_connection'] = self.check_visa_connection()
            
            if health_status['visa_connection']:
                self.log("✓ VISA connection successful")
            else:
                self.log("✗ VISA connection failed")
        else:
            self.log("✗ Network ping failed")
        
        # Determine overall status
        if all([health_status['network_ping'], health_status['web_interface'], health_status['visa_connection']]):
            health_status['overall_status'] = 'HEALTHY'
            self.log("🟢 Device Status: HEALTHY")
        elif health_status['network_ping']:
            health_status['overall_status'] = 'DEGRADED'
            self.log("🟡 Device Status: DEGRADED (network OK, services failing)")
        else:
            health_status['overall_status'] = 'OFFLINE'
            self.log("🔴 Device Status: OFFLINE")
        
        return health_status
    
    def attempt_recovery(self):
        """Attempt progressive recovery steps"""
        self.log("=== Starting Recovery Process ===")
        
        # Step 1: Soft reset via VISA
        self.log("Step 1: Attempting soft reset via VISA...")
        if self.soft_reset_visa():
            time.sleep(5)  # Wait for reset to complete
            if self.check_device_health()['overall_status'] == 'HEALTHY':
                self.log("✓ Recovery successful via soft reset")
                return True
        
        # Step 2: Network-based reset
        self.log("Step 2: Attempting network-based reset...")
        if self.hard_reset_network():
            time.sleep(10)  # Wait for network reset
            if self.check_device_health()['overall_status'] == 'HEALTHY':
                self.log("✓ Recovery successful via network reset")
                return True
        
        # Step 3: SNMP power cycle (if available)
        self.log("Step 3: Attempting SNMP power cycle...")
        if self.power_cycle_via_snmp():
            time.sleep(15)  # Wait for power cycle
            if self.check_device_health()['overall_status'] == 'HEALTHY':
                self.log("✓ Recovery successful via SNMP power cycle")
                return True
        
        self.log("✗ All automatic recovery attempts failed")
        self.log("Manual intervention required:")
        self.log("1. Physical power cycle of the device")
        self.log("2. Check network connections")
        self.log("3. Restart network switch/router if needed")
        
        return False

def main():
    parser = argparse.ArgumentParser(description='Keysight Power Meter Management Utility')
    parser.add_argument('action', choices=['health', 'reset', 'recover'], 
                       help='Action to perform: health check, reset, or full recovery')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    manager = KeysightManager()
    
    if args.action == 'health':
        health = manager.check_device_health()
        sys.exit(0 if health['overall_status'] == 'HEALTHY' else 1)
    
    elif args.action == 'reset':
        if manager.soft_reset_visa():
            manager.log("Reset completed successfully")
            sys.exit(0)
        else:
            manager.log("Reset failed")
            sys.exit(1)
    
    elif args.action == 'recover':
        if manager.attempt_recovery():
            manager.log("Recovery completed successfully")
            sys.exit(0)
        else:
            manager.log("Recovery failed - manual intervention required")
            sys.exit(1)

if __name__ == "__main__":
    main()