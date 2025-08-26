"""
Agilent Power Supply Equipment Class

Unified class for Agilent E364x series power supplies using VISA interface.
Implements BaseEquipment interface for consistent API.
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from base_equipment import BaseEquipment, EquipmentStatus, IOType, EquipmentError
import visa_instrument
from typing import Dict, Any, Optional, List


class AgilentPowerSupplyEquipment(BaseEquipment):
    """
    Agilent/Keysight Power Supply Equipment Class.
    
    Supports models: E3640A, E3646A, E3648A series
    """
    
    # Model definitions with channel and range information
    MODEL_DEFINITIONS = {
        'e3640a': {
            'num_output_channels': 1,
            'ranges': {
                'P8V': {'max_volts': 8, 'max_amps': 3},
                'P20V': {'max_volts': 20, 'max_amps': 1.5}
            }
        },
        'e3646a': {
            'num_output_channels': 2,
            'ranges': {
                'P8V': {'max_volts': 8, 'max_amps': 3},
                'P20V': {'max_volts': 20, 'max_amps': 1.5}
            }
        },
        'e3648a': {
            'num_output_channels': 2,
            'ranges': {
                'P8V': {'max_volts': 8, 'max_amps': 5},
                'P20V': {'max_volts': 20, 'max_amps': 2.5}
            }
        }
    }
    
    def __init__(self, visa_address: str, model: str = 'e3640a', equipment_name: str = "Agilent Power Supply"):
        """
        Initialize Agilent Power Supply.
        
        Args:
            visa_address: VISA resource string
            model: Power supply model (e3640a, e3646a, e3648a)
            equipment_name: Custom equipment name
        """
        connection_params = {
            'visa_address': visa_address,
            'timeout': 5000,
            'term_chars': '\n'
        }
        
        super().__init__(equipment_name, IOType.ETHERNET, connection_params)
        
        self.model = model.lower()
        self.model_info = self.MODEL_DEFINITIONS.get(self.model, self.MODEL_DEFINITIONS['e3640a'])
        
        # Power supply specific configuration
        self.default_config.update({
            'output_enabled': False,
            'voltage_channel_1': 0.0,
            'current_limit_channel_1': 0.1,
            'voltage_channel_2': 0.0 if self.model_info['num_output_channels'] > 1 else None,
            'current_limit_channel_2': 0.1 if self.model_info['num_output_channels'] > 1 else None,
            'range_channel_1': 'P8V',
            'range_channel_2': 'P8V' if self.model_info['num_output_channels'] > 1 else None,
            'over_voltage_protection': True,
            'over_current_protection': True
        })
        self.config.update(self.default_config)
        
        self._ps = None
        self._num_channels = self.model_info['num_output_channels']
    
    def connect(self) -> bool:
        """Establish VISA connection to Power Supply."""
        try:
            self._set_status(EquipmentStatus.CONNECTING)
            
            # Create VISA instrument connection
            self._ps = visa_instrument.VisaInstrument(
                self.connection_params['visa_address'],
                term_chars=self.connection_params['term_chars'],
                do_selftest=True,
                timeout_arg=self.connection_params['timeout'] / 1000
            )
            
            # Verify connection and get model info
            idn = self._ps.identification_query()
            if not idn:
                raise EquipmentError("Failed to get identification from Power Supply")
            
            # Initialize to safe state
            self._ps.set_output_state(False)  # Turn off output
            
            self._set_status(EquipmentStatus.READY)
            return True
            
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Connection failed: {str(e)}")
            return False
    
    def disconnect(self) -> bool:
        """Close VISA connection to Power Supply."""
        try:
            if self._ps:
                # Safe shutdown: turn off outputs
                self._ps.set_output_state(False)
                self._ps = None
            
            self._set_status(EquipmentStatus.DISCONNECTED)
            return True
            
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Disconnect failed: {str(e)}")
            return False
    
    def is_connected(self) -> bool:
        """Check if Power Supply is connected."""
        try:
            if not self._ps:
                return False
            
            # Try a simple query to verify connection
            self._ps.identification_query()
            return True
            
        except:
            return False
    
    def apply_config(self) -> bool:
        """Apply current configuration to Power Supply."""
        if not self.is_connected():
            self.last_error = "Power Supply not connected"
            return False
        
        try:
            # Configure Channel 1
            self._ps.set_voltage(self.config['voltage_channel_1'], channel=1)
            self._ps.set_current_limit(self.config['current_limit_channel_1'], channel=1)
            
            # Configure Channel 2 if available
            if self._num_channels > 1 and self.config['voltage_channel_2'] is not None:
                self._ps.set_voltage(self.config['voltage_channel_2'], channel=2)
                self._ps.set_current_limit(self.config['current_limit_channel_2'], channel=2)
            
            # Apply output state
            self._ps.set_output_state(self.config['output_enabled'])
            
            return True
            
        except Exception as e:
            self.last_error = f"Configuration failed: {str(e)}"
            return False
    
    def reset(self) -> bool:
        """Reset Power Supply to default state."""
        if not self.is_connected():
            return False
        
        try:
            # Turn off outputs first for safety
            self._ps.set_output_state(False)
            
            # Reset instrument
            self._ps.reset()
            self._ps.clear_status()
            self._ps.clear_error_stack()
            
            # Restore default configuration
            self.config.update(self.default_config)
            self.apply_config()
            
            return True
            
        except Exception as e:
            self.last_error = f"Reset failed: {str(e)}"
            return False
    
    def start_measurement(self) -> bool:
        """Start monitoring power supply outputs."""
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
        """Stop monitoring power supply outputs."""
        try:
            self._measurement_active = False
            self._set_status(EquipmentStatus.READY)
            return True
            
        except Exception as e:
            self.last_error = f"Stop measurement failed: {str(e)}"
            return False
    
    def measure(self) -> Optional[Dict[str, float]]:
        """Measure all output channels."""
        if not self.is_connected():
            return None
        
        try:
            measurements = {}
            
            # Measure Channel 1
            measurements['voltage_ch1'] = self._ps.measure_voltage(channel=1)
            measurements['current_ch1'] = self._ps.measure_current(channel=1)
            measurements['power_ch1'] = measurements['voltage_ch1'] * measurements['current_ch1']
            
            # Measure Channel 2 if available
            if self._num_channels > 1:
                measurements['voltage_ch2'] = self._ps.measure_voltage(channel=2)
                measurements['current_ch2'] = self._ps.measure_current(channel=2)
                measurements['power_ch2'] = measurements['voltage_ch2'] * measurements['current_ch2']
            
            # Store measurement
            self._add_measurement(measurements, 'V/A/W', {
                'channels': self._num_channels,
                'output_enabled': self.config['output_enabled']
            })
            
            return measurements
            
        except Exception as e:
            self.last_error = f"Measurement failed: {str(e)}"
            return None
    
    def self_test(self) -> bool:
        """Perform Power Supply self-test."""
        if not self.is_connected():
            return False
        
        try:
            result = self._ps.self_test()
            return result == "0"  # 0 = pass, 1 = fail
            
        except Exception as e:
            self.last_error = f"Self-test failed: {str(e)}"
            return False
    
    def get_identification(self) -> Dict[str, str]:
        """Get Power Supply identification information."""
        if not self.is_connected():
            return {}
        
        try:
            return self._ps.idn_info.copy()
        except:
            return {}
    
    # Power Supply specific methods
    def set_voltage(self, voltage: float, channel: int = 1) -> bool:
        """
        Set output voltage for specified channel.
        
        Args:
            voltage: Voltage in volts
            channel: Output channel (1 or 2)
        """
        if not self.is_connected():
            return False
        
        try:
            # Validate channel
            if channel < 1 or channel > self._num_channels:
                raise ValueError(f"Invalid channel {channel}")
            
            # Check voltage limits
            current_range = self.config[f'range_channel_{channel}']
            max_voltage = self.model_info['ranges'][current_range]['max_volts']
            if voltage > max_voltage:
                raise ValueError(f"Voltage {voltage}V exceeds maximum {max_voltage}V for range {current_range}")
            
            self._ps.set_voltage(voltage, channel=channel)
            self.config[f'voltage_channel_{channel}'] = voltage
            return True
            
        except Exception as e:
            self.last_error = f"Set voltage failed: {str(e)}"
            return False
    
    def set_current_limit(self, current: float, channel: int = 1) -> bool:
        """
        Set current limit for specified channel.
        
        Args:
            current: Current limit in amperes
            channel: Output channel (1 or 2)
        """
        if not self.is_connected():
            return False
        
        try:
            # Validate channel
            if channel < 1 or channel > self._num_channels:
                raise ValueError(f"Invalid channel {channel}")
            
            # Check current limits
            current_range = self.config[f'range_channel_{channel}']
            max_current = self.model_info['ranges'][current_range]['max_amps']
            if current > max_current:
                raise ValueError(f"Current {current}A exceeds maximum {max_current}A for range {current_range}")
            
            self._ps.set_current_limit(current, channel=channel)
            self.config[f'current_limit_channel_{channel}'] = current
            return True
            
        except Exception as e:
            self.last_error = f"Set current limit failed: {str(e)}"
            return False
    
    def enable_output(self, enabled: bool = True) -> bool:
        """Enable or disable power supply output."""
        if not self.is_connected():
            return False
        
        try:
            self._ps.set_output_state(enabled)
            self.config['output_enabled'] = enabled
            return True
            
        except Exception as e:
            self.last_error = f"Output control failed: {str(e)}"
            return False
    
    def get_output_status(self) -> Dict[str, Any]:
        """Get current output status for all channels."""
        if not self.is_connected():
            return {}
        
        try:
            status = {
                'output_enabled': self.config['output_enabled'],
                'num_channels': self._num_channels,
                'channel_1': {
                    'voltage_set': self.config['voltage_channel_1'],
                    'current_limit': self.config['current_limit_channel_1'],
                    'range': self.config['range_channel_1']
                }
            }
            
            if self._num_channels > 1:
                status['channel_2'] = {
                    'voltage_set': self.config['voltage_channel_2'],
                    'current_limit': self.config['current_limit_channel_2'],
                    'range': self.config['range_channel_2']
                }
            
            return status
            
        except Exception as e:
            self.last_error = f"Get status failed: {str(e)}"
            return {}
    
    def emergency_stop(self) -> bool:
        """Emergency shutdown of all outputs."""
        try:
            if self._ps:
                self._ps.set_output_state(False)
                self.config['output_enabled'] = False
            return True
        except:
            return False