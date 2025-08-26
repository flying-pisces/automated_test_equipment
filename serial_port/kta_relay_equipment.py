"""
KTA Relay Equipment Class

Unified class for KTA 223 relay controller using serial interface.
Implements BaseEquipment interface for consistent API.
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from base_equipment import BaseEquipment, EquipmentStatus, IOType, EquipmentError
import serial
from time import sleep
from typing import Dict, Any, Optional, List


class KTARelayEquipment(BaseEquipment):
    """
    KTA 223 Relay Controller Equipment Class.
    
    Controls multiple relay channels via serial interface.
    """
    
    # Relay state constants
    RELAY_STATE_OFF = 'connect_normal_closed'
    RELAY_STATE_ON = 'connect_normal_open'
    
    VOLTS_PER_COUNT = 0.0048875
    
    def __init__(self, com_port: str, baud_rate: int = 9600, equipment_name: str = "KTA Relay Controller"):
        """
        Initialize KTA Relay Controller.
        
        Args:
            com_port: Serial port (e.g., 'COM3', '/dev/ttyUSB0')
            baud_rate: Serial baud rate (default 9600)
            equipment_name: Custom equipment name
        """
        connection_params = {
            'com_port': com_port,
            'baud_rate': baud_rate,
            'timeout': 1.0,
            'parity': serial.PARITY_NONE,
            'stopbits': serial.STOPBITS_ONE,
            'bytesize': serial.EIGHTBITS
        }
        
        super().__init__(equipment_name, IOType.SERIAL, connection_params)
        
        # KTA-specific configuration
        self.default_config.update({
            'num_relays': 8,  # Standard KTA 223 has 8 relays
            'relay_states': [False] * 8,  # All relays initially off
            'voltage_monitoring': True,
            'auto_response': True
        })
        self.config.update(self.default_config)
        
        self._serial_port = None
        self._relay_labels = [f"Relay_{i+1}" for i in range(8)]
    
    def connect(self) -> bool:
        """Establish serial connection to KTA controller."""
        try:
            self._set_status(EquipmentStatus.CONNECTING)
            
            # Create serial connection
            self._serial_port = serial.Serial(
                port=self.connection_params['com_port'],
                baudrate=self.connection_params['baud_rate'],
                timeout=self.connection_params['timeout'],
                parity=self.connection_params['parity'],
                stopbits=self.connection_params['stopbits'],
                bytesize=self.connection_params['bytesize']
            )
            
            # Allow connection to stabilize
            sleep(0.5)
            
            # Test connection with identity query
            if not self._test_communication():
                raise EquipmentError("Failed to establish communication with KTA controller")
            
            self._set_status(EquipmentStatus.READY)
            return True
            
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Connection failed: {str(e)}")
            return False
    
    def disconnect(self) -> bool:
        """Close serial connection to KTA controller."""
        try:
            # Turn off all relays for safety
            self._turn_off_all_relays()
            
            if self._serial_port and self._serial_port.is_open:
                self._serial_port.close()
            self._serial_port = None
            
            self._set_status(EquipmentStatus.DISCONNECTED)
            return True
            
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Disconnect failed: {str(e)}")
            return False
    
    def is_connected(self) -> bool:
        """Check if KTA controller is connected."""
        try:
            if not self._serial_port or not self._serial_port.is_open:
                return False
            
            return self._test_communication()
            
        except:
            return False
    
    def apply_config(self) -> bool:
        """Apply current relay configuration."""
        if not self.is_connected():
            self.last_error = "KTA controller not connected"
            return False
        
        try:
            # Apply relay states
            for i, state in enumerate(self.config['relay_states']):
                self.set_relay_state(i + 1, state)
            
            return True
            
        except Exception as e:
            self.last_error = f"Configuration failed: {str(e)}"
            return False
    
    def reset(self) -> bool:
        """Reset KTA controller to default state."""
        if not self.is_connected():
            return False
        
        try:
            # Turn off all relays
            self._turn_off_all_relays()
            
            # Reset configuration
            self.config.update(self.default_config)
            self.apply_config()
            
            return True
            
        except Exception as e:
            self.last_error = f"Reset failed: {str(e)}"
            return False
    
    def start_measurement(self) -> bool:
        """Start monitoring relay states and voltages."""
        if not self.is_connected():
            return False
        
        try:
            self._measurement_active = True
            self._set_status(EquipmentStatus.MEASURING)
            return True
            
        except Exception as e:
            self._measurement_active = False
            self._set_status(EquipmentStatus.ERROR, f"Start measurement failed: {str(e)}")
            return False
    
    def stop_measurement(self) -> bool:
        """Stop monitoring relay states."""
        try:
            self._measurement_active = False
            self._set_status(EquipmentStatus.READY)
            return True
            
        except Exception as e:
            self.last_error = f"Stop measurement failed: {str(e)}"
            return False
    
    def measure(self) -> Optional[Dict[str, Any]]:
        """Measure current relay states and voltages."""
        if not self.is_connected():
            return None
        
        try:
            measurements = {
                'relay_states': self.get_all_relay_states(),
                'voltages': self.read_all_voltages() if self.config['voltage_monitoring'] else None
            }
            
            # Store measurement
            self._add_measurement(measurements, 'bool/V', {
                'num_relays': self.config['num_relays']
            })
            
            return measurements
            
        except Exception as e:
            self.last_error = f"Measurement failed: {str(e)}"
            return None
    
    def self_test(self) -> bool:
        """Perform KTA controller self-test."""
        if not self.is_connected():
            return False
        
        try:
            # Test communication
            if not self._test_communication():
                return False
            
            # Test relay switching
            original_states = self.get_all_relay_states()
            
            # Test each relay
            for i in range(self.config['num_relays']):
                # Turn relay on
                if not self.set_relay_state(i + 1, True):
                    return False
                sleep(0.1)
                
                # Verify state
                if not self.get_relay_state(i + 1):
                    return False
                
                # Turn relay off
                if not self.set_relay_state(i + 1, False):
                    return False
                sleep(0.1)
            
            # Restore original states
            for i, state in enumerate(original_states):
                self.set_relay_state(i + 1, state)
            
            return True
            
        except Exception as e:
            self.last_error = f"Self-test failed: {str(e)}"
            return False
    
    def get_identification(self) -> Dict[str, str]:
        """Get KTA controller identification."""
        return {
            'manufacturer': 'KTA',
            'model': '223',
            'serial_number': 'Unknown',
            'firmware': 'Unknown'
        }
    
    def _test_communication(self) -> bool:
        """Test basic communication with KTA controller."""
        try:
            # Send a simple command and check response
            self._send_command("getversion")
            response = self._read_response()
            return response is not None
        except:
            return False
    
    def _send_command(self, command: str):
        """Send command to KTA controller."""
        if not self._serial_port:
            raise EquipmentError("Serial port not open")
        
        cmd_bytes = (command + '\r\n').encode('ascii')
        self._serial_port.write(cmd_bytes)
        self._serial_port.flush()
    
    def _read_response(self, timeout: float = 1.0) -> Optional[str]:
        """Read response from KTA controller."""
        if not self._serial_port:
            return None
        
        self._serial_port.timeout = timeout
        try:
            response = self._serial_port.readline().decode('ascii').strip()
            return response if response else None
        except:
            return None
    
    def _turn_off_all_relays(self):
        """Turn off all relays for safety."""
        try:
            for i in range(self.config['num_relays']):
                self.set_relay_state(i + 1, False)
        except:
            pass  # Ignore errors during shutdown
    
    # KTA-specific methods
    def set_relay_state(self, relay_number: int, state: bool) -> bool:
        """
        Set state of specific relay.
        
        Args:
            relay_number: Relay number (1-8)
            state: True for ON, False for OFF
        """
        if not self.is_connected():
            return False
        
        try:
            if relay_number < 1 or relay_number > self.config['num_relays']:
                raise ValueError(f"Invalid relay number {relay_number}")
            
            state_str = self.RELAY_STATE_ON if state else self.RELAY_STATE_OFF
            command = f"setrelaystate {relay_number} {state_str}"
            
            self._send_command(command)
            response = self._read_response()
            
            if response and "OK" in response:
                self.config['relay_states'][relay_number - 1] = state
                return True
            
            return False
            
        except Exception as e:
            self.last_error = f"Set relay state failed: {str(e)}"
            return False
    
    def get_relay_state(self, relay_number: int) -> Optional[bool]:
        """
        Get state of specific relay.
        
        Args:
            relay_number: Relay number (1-8)
            
        Returns:
            True if relay is ON, False if OFF, None if error
        """
        if not self.is_connected():
            return None
        
        try:
            if relay_number < 1 or relay_number > self.config['num_relays']:
                raise ValueError(f"Invalid relay number {relay_number}")
            
            command = f"getrelaystate {relay_number}"
            self._send_command(command)
            response = self._read_response()
            
            if response:
                return self.RELAY_STATE_ON in response
            
            return None
            
        except Exception as e:
            self.last_error = f"Get relay state failed: {str(e)}"
            return None
    
    def get_all_relay_states(self) -> List[bool]:
        """Get states of all relays."""
        states = []
        for i in range(self.config['num_relays']):
            state = self.get_relay_state(i + 1)
            states.append(state if state is not None else False)
        return states
    
    def read_voltage(self, channel: int) -> Optional[float]:
        """
        Read voltage from analog input channel.
        
        Args:
            channel: Channel number
            
        Returns:
            Voltage in volts or None if error
        """
        if not self.is_connected():
            return None
        
        try:
            command = f"readvoltage {channel}"
            self._send_command(command)
            response = self._read_response()
            
            if response and response.isdigit():
                counts = int(response)
                voltage = counts * self.VOLTS_PER_COUNT
                return voltage
            
            return None
            
        except Exception as e:
            self.last_error = f"Read voltage failed: {str(e)}"
            return None
    
    def read_all_voltages(self) -> List[Optional[float]]:
        """Read voltages from all available channels."""
        voltages = []
        # Assume 4 voltage channels (typical for KTA 223)
        for i in range(4):
            voltage = self.read_voltage(i)
            voltages.append(voltage)
        return voltages
    
    def set_relay_labels(self, labels: List[str]):
        """Set custom labels for relays."""
        if len(labels) == self.config['num_relays']:
            self._relay_labels = labels.copy()
    
    def get_relay_labels(self) -> List[str]:
        """Get current relay labels."""
        return self._relay_labels.copy()