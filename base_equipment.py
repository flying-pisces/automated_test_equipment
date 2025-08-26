"""
Base Equipment Class - Universal Interface for All Instruments

This module provides a common interface for all laboratory equipment,
ensuring consistent method signatures and behavior across all instrument types.
"""

from abc import ABC, abstractmethod
from enum import Enum
from typing import Dict, Any, Optional, List
import json
import time
from datetime import datetime


class EquipmentStatus(Enum):
    """Standard equipment status states"""
    DISCONNECTED = "disconnected"
    CONNECTING = "connecting" 
    READY = "ready"
    MEASURING = "measuring"
    ERROR = "error"
    BUSY = "busy"


class IOType(Enum):
    """Equipment I/O interface types"""
    ETHERNET = "ethernet"
    SERIAL = "serial"
    USB = "usb" 
    SPECIAL = "special"


class EquipmentError(Exception):
    """Base exception for all equipment errors"""
    pass


class BaseEquipment(ABC):
    """
    Abstract base class for all laboratory equipment.
    
    Provides standard interface methods that all equipment must implement:
    - Connection management (connect, disconnect, is_connected)
    - Configuration (get_config, set_config, reset)
    - Measurement operations (start, stop, measure)
    - Data handling (export_data, get_status)
    - Error handling and logging
    """
    
    def __init__(self, equipment_name: str, io_type: IOType, connection_params: Dict[str, Any]):
        """
        Initialize base equipment.
        
        Args:
            equipment_name: Human-readable equipment name
            io_type: Type of I/O interface (ethernet, serial, usb, special)
            connection_params: Connection parameters (address, port, etc.)
        """
        self.equipment_name = equipment_name
        self.io_type = io_type
        self.connection_params = connection_params
        self.status = EquipmentStatus.DISCONNECTED
        self.last_error = None
        self.measurement_data = []
        self.config = {}
        self._connection = None
        self._measurement_active = False
        
        # Standard configuration attributes - override in subclasses
        self.default_config = {
            'timeout': 5.0,
            'auto_range': True,
            'measurement_units': 'SI',
            'sample_rate': 1.0,
            'precision': 6
        }
        self.config.update(self.default_config)
    
    # Connection Management Methods
    @abstractmethod
    def connect(self) -> bool:
        """
        Establish connection to equipment.
        
        Returns:
            bool: True if connection successful, False otherwise
        """
        pass
    
    @abstractmethod
    def disconnect(self) -> bool:
        """
        Close connection to equipment.
        
        Returns:
            bool: True if disconnection successful, False otherwise
        """
        pass
    
    @abstractmethod
    def is_connected(self) -> bool:
        """
        Check if equipment is connected.
        
        Returns:
            bool: True if connected, False otherwise
        """
        pass
    
    # Configuration Methods
    def get_config(self) -> Dict[str, Any]:
        """
        Get current equipment configuration.
        
        Returns:
            Dict containing current configuration parameters
        """
        return self.config.copy()
    
    def set_config(self, config_params: Dict[str, Any]) -> bool:
        """
        Update equipment configuration.
        
        Args:
            config_params: Dictionary of configuration parameters to update
            
        Returns:
            bool: True if configuration updated successfully
        """
        try:
            self.config.update(config_params)
            return self.apply_config()
        except Exception as e:
            self.last_error = f"Configuration error: {str(e)}"
            return False
    
    @abstractmethod
    def apply_config(self) -> bool:
        """
        Apply current configuration to equipment.
        
        Returns:
            bool: True if configuration applied successfully
        """
        pass
    
    @abstractmethod
    def reset(self) -> bool:
        """
        Reset equipment to default state.
        
        Returns:
            bool: True if reset successful
        """
        pass
    
    # Measurement Methods
    @abstractmethod
    def start_measurement(self) -> bool:
        """
        Start measurement process.
        
        Returns:
            bool: True if measurement started successfully
        """
        pass
    
    @abstractmethod
    def stop_measurement(self) -> bool:
        """
        Stop measurement process.
        
        Returns:
            bool: True if measurement stopped successfully
        """
        pass
    
    @abstractmethod
    def measure(self) -> Any:
        """
        Perform a single measurement.
        
        Returns:
            Measurement result (type depends on equipment)
        """
        pass
    
    def is_measuring(self) -> bool:
        """
        Check if measurement is in progress.
        
        Returns:
            bool: True if measurement active
        """
        return self._measurement_active
    
    # Data Management Methods
    def get_measurement_data(self) -> List[Dict[str, Any]]:
        """
        Get stored measurement data.
        
        Returns:
            List of measurement records with timestamps
        """
        return self.measurement_data.copy()
    
    def clear_data(self):
        """Clear stored measurement data."""
        self.measurement_data.clear()
    
    def export_data(self, filename: str, format: str = 'json') -> bool:
        """
        Export measurement data to file.
        
        Args:
            filename: Output filename
            format: Export format ('json', 'csv', 'txt')
            
        Returns:
            bool: True if export successful
        """
        try:
            if format.lower() == 'json':
                with open(filename, 'w') as f:
                    json.dump({
                        'equipment': self.equipment_name,
                        'timestamp': datetime.now().isoformat(),
                        'config': self.config,
                        'data': self.measurement_data
                    }, f, indent=2)
            elif format.lower() == 'csv':
                import csv
                with open(filename, 'w', newline='') as f:
                    if self.measurement_data:
                        writer = csv.DictWriter(f, fieldnames=self.measurement_data[0].keys())
                        writer.writeheader()
                        writer.writerows(self.measurement_data)
            else:  # txt format
                with open(filename, 'w') as f:
                    f.write(f"Equipment: {self.equipment_name}\n")
                    f.write(f"Export Time: {datetime.now()}\n")
                    f.write(f"Configuration: {self.config}\n\n")
                    for record in self.measurement_data:
                        f.write(f"{record}\n")
            return True
        except Exception as e:
            self.last_error = f"Export error: {str(e)}"
            return False
    
    # Status and Error Methods
    def get_status(self) -> Dict[str, Any]:
        """
        Get equipment status information.
        
        Returns:
            Dict containing status, connection state, and error info
        """
        return {
            'name': self.equipment_name,
            'status': self.status.value,
            'connected': self.is_connected(),
            'measuring': self.is_measuring(),
            'io_type': self.io_type.value,
            'last_error': self.last_error,
            'data_points': len(self.measurement_data)
        }
    
    def get_last_error(self) -> Optional[str]:
        """
        Get last error message.
        
        Returns:
            Last error message or None if no error
        """
        return self.last_error
    
    def clear_error(self):
        """Clear last error message."""
        self.last_error = None
    
    # Utility Methods
    def _add_measurement(self, value: Any, units: str = "", metadata: Dict = None):
        """
        Add measurement to data store.
        
        Args:
            value: Measured value
            units: Measurement units
            metadata: Additional measurement metadata
        """
        record = {
            'timestamp': datetime.now().isoformat(),
            'value': value,
            'units': units
        }
        if metadata:
            record.update(metadata)
        self.measurement_data.append(record)
    
    def _set_status(self, status: EquipmentStatus, error_msg: str = None):
        """
        Update equipment status.
        
        Args:
            status: New status
            error_msg: Error message if status is ERROR
        """
        self.status = status
        if error_msg:
            self.last_error = error_msg
    
    @abstractmethod
    def self_test(self) -> bool:
        """
        Perform equipment self-test.
        
        Returns:
            bool: True if self-test passes
        """
        pass
    
    @abstractmethod
    def get_identification(self) -> Dict[str, str]:
        """
        Get equipment identification information.
        
        Returns:
            Dict with manufacturer, model, serial number, firmware version
        """
        pass
    
    def __str__(self) -> str:
        """String representation of equipment."""
        return f"{self.equipment_name} ({self.io_type.value}) - {self.status.value}"
    
    def __repr__(self) -> str:
        """Detailed string representation."""
        return f"<{self.__class__.__name__}: {self.equipment_name}, {self.status.value}>"