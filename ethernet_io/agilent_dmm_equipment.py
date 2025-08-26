"""
Agilent Digital Multimeter Equipment Class

Unified class for Agilent 34461A and compatible DMMs using VISA interface.
Implements BaseEquipment interface for consistent API.
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from base_equipment import BaseEquipment, EquipmentStatus, IOType, EquipmentError
import visa_instrument
from typing import Dict, Any, Optional


class AgilentDMMEquipment(BaseEquipment):
    """
    Agilent/Keysight Digital Multimeter Equipment Class.
    
    Supports models: 34461A, 34460A, 34410A, 34411A
    """
    
    # Measurement function constants
    MEASUREMENT_FUNCTIONS = {
        'voltage_dc': "VOLTage:DC",
        'voltage_ac': "VOLTage:AC", 
        'current_dc': "CURRent:DC",
        'current_ac': "CURRent:AC",
        'resistance': "RESistance",
        'fresistance': "FRESistance",
        'capacitance': "CAPacitance",
        'continuity': "CONTinuity",
        'diode': "DIODe",
        'frequency': "FREQuency",
        'period': "PERiod",
        'temperature': "TEMPerature"
    }
    
    def __init__(self, visa_address: str, equipment_name: str = "Agilent DMM"):
        """
        Initialize Agilent DMM.
        
        Args:
            visa_address: VISA resource string (e.g., "TCPIP::192.168.1.100::INSTR")
            equipment_name: Custom equipment name
        """
        connection_params = {
            'visa_address': visa_address,
            'timeout': 5000,
            'term_chars': '\n'
        }
        
        super().__init__(equipment_name, IOType.ETHERNET, connection_params)
        
        # DMM-specific configuration
        self.default_config.update({
            'measurement_function': 'voltage_dc',
            'auto_range': True,
            'range': 10.0,
            'aperture': 0.02,  # Integration time in seconds
            'trigger_source': 'immediate',
            'sample_count': 1,
            'display_enabled': True
        })
        self.config.update(self.default_config)
        
        self._dmm = None
        self._current_function = 'voltage_dc'
    
    def connect(self) -> bool:
        """Establish VISA connection to DMM."""
        try:
            self._set_status(EquipmentStatus.CONNECTING)
            
            # Create VISA instrument connection
            self._dmm = visa_instrument.VisaInstrument(
                self.connection_params['visa_address'],
                term_chars=self.connection_params['term_chars'],
                do_selftest=True,
                timeout_arg=self.connection_params['timeout'] / 1000
            )
            
            # Verify connection with identification
            idn = self._dmm.identification_query()
            if not idn:
                raise EquipmentError("Failed to get identification from DMM")
            
            self._set_status(EquipmentStatus.READY)
            return True
            
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Connection failed: {str(e)}")
            return False
    
    def disconnect(self) -> bool:
        """Close VISA connection to DMM."""
        try:
            if self._dmm:
                # No explicit close method in visa_instrument, but connection will be closed
                self._dmm = None
            
            self._set_status(EquipmentStatus.DISCONNECTED)
            return True
            
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Disconnect failed: {str(e)}")
            return False
    
    def is_connected(self) -> bool:
        """Check if DMM is connected."""
        try:
            if not self._dmm:
                return False
            
            # Try a simple query to verify connection
            self._dmm.identification_query()
            return True
            
        except:
            return False
    
    def apply_config(self) -> bool:
        """Apply current configuration to DMM."""
        if not self.is_connected():
            self.last_error = "DMM not connected"
            return False
        
        try:
            # Set measurement function
            func = self.MEASUREMENT_FUNCTIONS.get(self.config['measurement_function'])
            if func:
                self._dmm.set_measurement_func(func)
                self._current_function = self.config['measurement_function']
            
            # Set range
            if not self.config['auto_range']:
                if 'voltage' in self._current_function:
                    self._dmm.set_range(self.config['range'])
                elif 'current' in self._current_function:
                    self._dmm.set_current_range(self.config['range'])
            
            return True
            
        except Exception as e:
            self.last_error = f"Configuration failed: {str(e)}"
            return False
    
    def reset(self) -> bool:
        """Reset DMM to default state."""
        if not self.is_connected():
            return False
        
        try:
            self._dmm.reset()
            self._dmm.clear_status()
            self._dmm.clear_error_stack()
            
            # Restore default configuration
            self.config.update(self.default_config)
            self.apply_config()
            
            return True
            
        except Exception as e:
            self.last_error = f"Reset failed: {str(e)}"
            return False
    
    def start_measurement(self) -> bool:
        """Start continuous measurement logging."""
        if not self.is_connected():
            return False
        
        try:
            self._measurement_active = True
            self._set_status(EquipmentStatus.MEASURING)
            
            # Configure logging parameters
            trigger_delay = 0.1
            sample_count = self.config.get('sample_count', 100)
            sample_interval = 1.0 / self.config.get('sample_rate', 1.0)
            
            self._dmm.start_logging(trigger_delay, sample_count, sample_interval)
            return True
            
        except Exception as e:
            self._measurement_active = False
            self._set_status(EquipmentStatus.ERROR, f"Start measurement failed: {str(e)}")
            return False
    
    def stop_measurement(self) -> bool:
        """Stop measurement logging."""
        try:
            if self._dmm:
                # Send abort command to stop measurement
                self._dmm.write("ABOR")
                
            self._measurement_active = False
            self._set_status(EquipmentStatus.READY)
            return True
            
        except Exception as e:
            self.last_error = f"Stop measurement failed: {str(e)}"
            return False
    
    def measure(self) -> Optional[float]:
        """Perform single measurement based on current function."""
        if not self.is_connected():
            return None
        
        try:
            # Ensure correct configuration is applied
            self.apply_config()
            
            # Perform measurement based on current function
            if 'voltage' in self._current_function:
                result = self._dmm.measure_voltage()
            elif 'current' in self._current_function:
                result = self._dmm.measure_current()
            else:
                # Use generic measurement query
                scpi_func = self.MEASUREMENT_FUNCTIONS[self._current_function]
                result = self._dmm.ask_for_value(f"MEAS:{scpi_func}?")
            
            # Store measurement with metadata
            units = self._get_measurement_units()
            self._add_measurement(result, units, {
                'function': self._current_function,
                'range': self.config.get('range', 'auto')
            })
            
            return result
            
        except Exception as e:
            self.last_error = f"Measurement failed: {str(e)}"
            return None
    
    def self_test(self) -> bool:
        """Perform DMM self-test."""
        if not self.is_connected():
            return False
        
        try:
            result = self._dmm.self_test()
            return result == "0"  # 0 = pass, 1 = fail
            
        except Exception as e:
            self.last_error = f"Self-test failed: {str(e)}"
            return False
    
    def get_identification(self) -> Dict[str, str]:
        """Get DMM identification information."""
        if not self.is_connected():
            return {}
        
        try:
            return self._dmm.idn_info.copy()
        except:
            return {}
    
    def _get_measurement_units(self) -> str:
        """Get appropriate units for current measurement function."""
        units_map = {
            'voltage_dc': 'V',
            'voltage_ac': 'V',
            'current_dc': 'A', 
            'current_ac': 'A',
            'resistance': 'Ω',
            'fresistance': 'Ω',
            'capacitance': 'F',
            'frequency': 'Hz',
            'period': 's',
            'temperature': '°C',
            'continuity': 'Ω',
            'diode': 'V'
        }
        return units_map.get(self._current_function, '')
    
    # DMM-specific methods
    def measure_voltage_dc(self, channel: int = None) -> Optional[float]:
        """Measure DC voltage."""
        self.config['measurement_function'] = 'voltage_dc'
        return self.measure()
    
    def measure_voltage_ac(self, channel: int = None) -> Optional[float]:
        """Measure AC voltage."""
        self.config['measurement_function'] = 'voltage_ac'
        return self.measure()
    
    def measure_current_dc(self, channel: int = None) -> Optional[float]:
        """Measure DC current."""
        self.config['measurement_function'] = 'current_dc'
        return self.measure()
    
    def measure_current_ac(self, channel: int = None) -> Optional[float]:
        """Measure AC current.""" 
        self.config['measurement_function'] = 'current_ac'
        return self.measure()
    
    def measure_resistance(self) -> Optional[float]:
        """Measure resistance (2-wire)."""
        self.config['measurement_function'] = 'resistance'
        return self.measure()
    
    def measure_resistance_4wire(self) -> Optional[float]:
        """Measure resistance (4-wire)."""
        self.config['measurement_function'] = 'fresistance'
        return self.measure()
    
    def set_range(self, measurement_range: float):
        """Set measurement range and disable auto-ranging."""
        self.config['range'] = measurement_range
        self.config['auto_range'] = False
        self.apply_config()
    
    def set_auto_range(self, enabled: bool = True):
        """Enable/disable auto-ranging."""
        self.config['auto_range'] = enabled
        self.apply_config()