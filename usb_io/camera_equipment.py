"""
Camera Equipment Class

Unified class for Point Grey/FLIR cameras using PySpin SDK.
Implements BaseEquipment interface for consistent API.
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from base_equipment import BaseEquipment, EquipmentStatus, IOType, EquipmentError
from typing import Dict, Any, Optional, List
import time

# Import PySpin if available
try:
    import PySpin
    PYSPIN_AVAILABLE = True
except ImportError:
    PYSPIN_AVAILABLE = False
    PySpin = None


class CameraEquipment(BaseEquipment):
    """
    Point Grey/FLIR Camera Equipment Class using PySpin SDK.
    
    Supports Spinnaker SDK compatible cameras.
    """
    
    def __init__(self, camera_index: int = 0, equipment_name: str = "FLIR Camera"):
        """
        Initialize Camera.
        
        Args:
            camera_index: Camera index (0 for first camera)
            equipment_name: Custom equipment name
        """
        if not PYSPIN_AVAILABLE:
            raise EquipmentError("PySpin SDK not available. Install Spinnaker SDK.")
        
        connection_params = {
            'camera_index': camera_index,
            'sdk_version': None
        }
        
        super().__init__(equipment_name, IOType.USB, connection_params)
        
        # Camera-specific configuration
        self.default_config.update({
            'trigger_mode': 'software',  # 'software', 'hardware', 'off'
            'exposure_time': 10000,  # microseconds
            'gain': 0.0,  # dB
            'frame_rate': 30.0,  # fps
            'pixel_format': 'Mono8',
            'image_width': 1920,
            'image_height': 1080,
            'acquisition_mode': 'continuous',  # 'continuous', 'single_frame'
            'buffer_handling_mode': 'newest_only'
        })
        self.config.update(self.default_config)
        
        self._system = None
        self._cam_list = None
        self._camera = None
        self._is_acquiring = False
        self._images_captured = []
    
    def connect(self) -> bool:
        """Establish connection to camera."""
        try:
            self._set_status(EquipmentStatus.CONNECTING)
            
            # Initialize Spinnaker system
            self._system = PySpin.System.GetInstance()
            
            # Get camera list
            self._cam_list = self._system.GetCameras()
            
            if self._cam_list.GetSize() == 0:
                raise EquipmentError("No cameras detected")
            
            # Get specified camera
            camera_index = self.connection_params['camera_index']
            if camera_index >= self._cam_list.GetSize():
                raise EquipmentError(f"Camera index {camera_index} not available")
            
            self._camera = self._cam_list[camera_index]
            
            # Initialize camera
            self._camera.Init()
            
            # Get SDK version info
            version = self._system.GetLibraryVersion()
            self.connection_params['sdk_version'] = f"{version.major}.{version.minor}.{version.type}.{version.build}"
            
            self._set_status(EquipmentStatus.READY)
            return True
            
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Connection failed: {str(e)}")
            return False
    
    def disconnect(self) -> bool:
        """Close connection to camera."""
        try:
            # Stop acquisition if running
            if self._is_acquiring:
                self.stop_measurement()
            
            # Deinitialize camera
            if self._camera:
                self._camera.DeInit()
                self._camera = None
            
            # Clear camera list
            if self._cam_list:
                self._cam_list.Clear()
                self._cam_list = None
            
            # Release system
            if self._system:
                self._system.ReleaseInstance()
                self._system = None
            
            self._set_status(EquipmentStatus.DISCONNECTED)
            return True
            
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Disconnect failed: {str(e)}")
            return False
    
    def is_connected(self) -> bool:
        """Check if camera is connected."""
        try:
            return (self._camera is not None and 
                   self._system is not None and
                   PySpin.IsAvailable(self._camera.GetNodeMap().GetNode('DeviceID')))\n            
        except:
            return False
    
    def apply_config(self) -> bool:
        """Apply current configuration to camera."""
        if not self.is_connected():
            self.last_error = "Camera not connected"
            return False
        
        try:
            nodemap = self._camera.GetNodeMap()
            
            # Configure trigger mode
            self._configure_trigger()
            
            # Set exposure time
            exposure_node = PySpin.CFloatPtr(nodemap.GetNode('ExposureTime'))
            if PySpin.IsAvailable(exposure_node) and PySpin.IsWritable(exposure_node):
                exposure_node.SetValue(self.config['exposure_time'])
            
            # Set gain
            gain_node = PySpin.CFloatPtr(nodemap.GetNode('Gain'))
            if PySpin.IsAvailable(gain_node) and PySpin.IsWritable(gain_node):
                gain_node.SetValue(self.config['gain'])
            
            # Set frame rate (if not using hardware trigger)
            if self.config['trigger_mode'] != 'hardware':
                fps_enable_node = PySpin.CBooleanPtr(nodemap.GetNode('AcquisitionFrameRateEnable'))
                if PySpin.IsAvailable(fps_enable_node) and PySpin.IsWritable(fps_enable_node):
                    fps_enable_node.SetValue(True)
                
                fps_node = PySpin.CFloatPtr(nodemap.GetNode('AcquisitionFrameRate'))
                if PySpin.IsAvailable(fps_node) and PySpin.IsWritable(fps_node):
                    fps_node.SetValue(self.config['frame_rate'])
            
            # Set pixel format
            pixel_format_node = PySpin.CEnumerationPtr(nodemap.GetNode('PixelFormat'))
            if PySpin.IsAvailable(pixel_format_node) and PySpin.IsWritable(pixel_format_node):
                pixel_format_entry = pixel_format_node.GetEntryByName(self.config['pixel_format'])
                if PySpin.IsAvailable(pixel_format_entry) and PySpin.IsReadable(pixel_format_entry):
                    pixel_format_node.SetIntValue(pixel_format_entry.GetValue())
            
            return True
            
        except Exception as e:
            self.last_error = f"Configuration failed: {str(e)}"
            return False
    
    def reset(self) -> bool:
        """Reset camera to default state."""
        if not self.is_connected():
            return False
        
        try:
            # Stop acquisition if running
            if self._is_acquiring:
                self.stop_measurement()
            
            # Reset configuration
            self.config.update(self.default_config)
            self.apply_config()
            
            # Clear captured images
            self._images_captured.clear()
            
            return True
            
        except Exception as e:
            self.last_error = f"Reset failed: {str(e)}"
            return False
    
    def start_measurement(self) -> bool:
        """Start image acquisition."""
        if not self.is_connected():
            return False
        
        try:
            # Apply current configuration
            self.apply_config()
            
            # Start acquisition
            self._camera.BeginAcquisition()
            self._is_acquiring = True
            self._measurement_active = True
            self._set_status(EquipmentStatus.MEASURING)
            
            return True
            
        except Exception as e:
            self._measurement_active = False
            self._is_acquiring = False
            self._set_status(EquipmentStatus.ERROR, f"Start acquisition failed: {str(e)}")
            return False
    
    def stop_measurement(self) -> bool:
        """Stop image acquisition."""
        try:
            if self._is_acquiring and self._camera:
                self._camera.EndAcquisition()
                self._is_acquiring = False
            
            self._measurement_active = False
            self._set_status(EquipmentStatus.READY)
            return True
            
        except Exception as e:
            self.last_error = f"Stop acquisition failed: {str(e)}"
            return False
    
    def measure(self) -> Optional[Any]:
        """Capture a single image."""
        if not self.is_connected():
            return None
        
        try:
            if not self._is_acquiring:
                # For single frame capture
                self._camera.BeginAcquisition()
                single_frame = True
            else:
                single_frame = False
            
            # Trigger image capture if using software trigger
            if self.config['trigger_mode'] == 'software':
                trigger_node = PySpin.CCommandPtr(self._camera.GetNodeMap().GetNode('TriggerSoftware'))
                if PySpin.IsAvailable(trigger_node) and PySpin.IsWritable(trigger_node):
                    trigger_node.Execute()
            
            # Get next image
            image_result = self._camera.GetNextImage(1000)  # 1 second timeout
            
            if image_result.IsIncomplete():
                self.last_error = f"Image incomplete: {image_result.GetImageStatus()}"
                return None
            
            # Convert image to numpy array or save
            image_data = {
                'timestamp': time.time(),
                'width': image_result.GetWidth(),
                'height': image_result.GetHeight(),
                'pixel_format': self.config['pixel_format'],
                'frame_id': image_result.GetFrameID(),
                'data': image_result.GetNDArray() if hasattr(image_result, 'GetNDArray') else None
            }
            
            # Store measurement
            self._add_measurement(image_data, 'pixels', {
                'capture_mode': self.config['trigger_mode'],
                'exposure_time': self.config['exposure_time']
            })
            
            # Release image
            image_result.Release()
            
            if single_frame:
                self._camera.EndAcquisition()
            
            return image_data
            
        except Exception as e:
            self.last_error = f"Image capture failed: {str(e)}"
            return None
    
    def self_test(self) -> bool:
        """Perform camera self-test."""
        if not self.is_connected():
            return False
        
        try:
            # Test basic image capture
            original_acquiring = self._is_acquiring
            
            # Capture test image
            test_image = self.measure()
            
            # Restore acquisition state
            if original_acquiring and not self._is_acquiring:
                self.start_measurement()
            
            return test_image is not None
            
        except Exception as e:
            self.last_error = f"Self-test failed: {str(e)}"
            return False
    
    def get_identification(self) -> Dict[str, str]:
        """Get camera identification information."""
        if not self.is_connected():
            return {}
        
        try:
            nodemap = self._camera.GetNodeMap()
            
            # Get device information
            device_vendor = self._get_node_value(nodemap, 'DeviceVendorName', 'Unknown')
            device_model = self._get_node_value(nodemap, 'DeviceModelName', 'Unknown')
            device_serial = self._get_node_value(nodemap, 'DeviceSerialNumber', 'Unknown')
            device_version = self._get_node_value(nodemap, 'DeviceVersion', 'Unknown')
            
            return {
                'manufacturer': device_vendor,
                'model': device_model,
                'serial_number': device_serial,
                'firmware': device_version
            }
            
        except:
            return {}
    
    def _configure_trigger(self):
        """Configure camera trigger settings."""
        try:
            nodemap = self._camera.GetNodeMap()
            
            # Get trigger mode node
            trigger_mode_node = PySpin.CEnumerationPtr(nodemap.GetNode('TriggerMode'))
            if not PySpin.IsAvailable(trigger_mode_node) or not PySpin.IsWritable(trigger_mode_node):
                return False
            
            if self.config['trigger_mode'] == 'off':
                # Turn off triggering
                trigger_mode_off = trigger_mode_node.GetEntryByName('Off')
                trigger_mode_node.SetIntValue(trigger_mode_off.GetValue())
                
            else:
                # Configure trigger source
                trigger_source_node = PySpin.CEnumerationPtr(nodemap.GetNode('TriggerSource'))
                if PySpin.IsAvailable(trigger_source_node) and PySpin.IsWritable(trigger_source_node):
                    if self.config['trigger_mode'] == 'software':
                        trigger_source_entry = trigger_source_node.GetEntryByName('Software')
                    else:  # hardware
                        trigger_source_entry = trigger_source_node.GetEntryByName('Line0')  # or appropriate hardware line
                    
                    if PySpin.IsAvailable(trigger_source_entry):
                        trigger_source_node.SetIntValue(trigger_source_entry.GetValue())
                
                # Turn on triggering
                trigger_mode_on = trigger_mode_node.GetEntryByName('On')
                trigger_mode_node.SetIntValue(trigger_mode_on.GetValue())
            
            return True
            
        except Exception as e:
            self.last_error = f"Trigger configuration failed: {str(e)}"
            return False
    
    def _get_node_value(self, nodemap, node_name: str, default_value: str = '') -> str:
        """Get string value from camera node."""
        try:
            node = PySpin.CStringPtr(nodemap.GetNode(node_name))
            if PySpin.IsAvailable(node) and PySpin.IsReadable(node):
                return node.GetValue()
        except:
            pass
        return default_value
    
    # Camera-specific methods
    def capture_image(self, filename: Optional[str] = None) -> Optional[Dict[str, Any]]:
        """
        Capture single image and optionally save to file.
        
        Args:
            filename: Optional filename to save image
            
        Returns:
            Image data dictionary
        """
        image_data = self.measure()
        
        if image_data and filename and 'data' in image_data:
            try:
                # Save image using appropriate format
                import cv2
                cv2.imwrite(filename, image_data['data'])
                image_data['saved_filename'] = filename
            except ImportError:
                self.last_error = "OpenCV not available for image saving"
            except Exception as e:
                self.last_error = f"Image save failed: {str(e)}"
        
        return image_data
    
    def get_camera_info(self) -> Dict[str, Any]:
        """Get comprehensive camera information."""
        if not self.is_connected():
            return {}
        
        info = self.get_identification()
        info.update({
            'sdk_version': self.connection_params.get('sdk_version', 'Unknown'),
            'current_config': self.get_config(),
            'is_acquiring': self._is_acquiring,
            'images_captured': len(self.measurement_data)
        })
        
        return info