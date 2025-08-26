#!/usr/bin/env python3
"""
Example Usage of Universal Equipment Controller

This script demonstrates how to use the new unified equipment classes
and GUI system for laboratory instruments.
"""

import sys
import os

# Add current directory to path for imports
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from equipment_controller import main as run_main_controller
from universal_equipment_gui import create_equipment_gui

# Import specific equipment classes
from ethernet_io.agilent_dmm_equipment import AgilentDMMEquipment
from ethernet_io.agilent_power_supply_equipment import AgilentPowerSupplyEquipment
from serial_port.kta_relay_equipment import KTARelayEquipment
from usb_io.camera_equipment import CameraEquipment


def example_single_equipment():
    """Example: Using a single equipment with GUI."""
    print("=== Single Equipment Example ===")
    
    # Create DMM equipment
    dmm = AgilentDMMEquipment(
        visa_address="TCPIP::192.168.1.100::INSTR",
        equipment_name="Lab DMM"
    )
    
    # Option 1: Use without GUI (programmatic control)
    print("Connecting to DMM...")
    if dmm.connect():
        print("Connected successfully!")
        
        # Configure measurement
        dmm.set_config({
            'measurement_function': 'voltage_dc',
            'auto_range': True
        })
        
        # Take measurement
        voltage = dmm.measure()
        if voltage is not None:
            print(f"Measured voltage: {voltage} V")
        
        # Export data
        dmm.export_data("dmm_data.json", "json")
        
        dmm.disconnect()
        print("Disconnected.")
    else:
        print(f"Connection failed: {dmm.get_last_error()}")
    
    # Option 2: Use with GUI
    print("\\nLaunching GUI for DMM control...")
    gui = create_equipment_gui(dmm)
    # gui.run()  # Uncomment to show GUI


def example_multiple_equipment():
    """Example: Managing multiple equipment programmatically."""
    print("\\n=== Multiple Equipment Example ===")
    
    # Create multiple equipment instances
    equipment_list = []
    
    # DMM
    dmm = AgilentDMMEquipment("TCPIP::192.168.1.100::INSTR", "DMM-1")
    equipment_list.append(dmm)
    
    # Power Supply
    ps = AgilentPowerSupplyEquipment(
        visa_address="TCPIP::192.168.1.101::INSTR",
        model="e3646a",
        equipment_name="Power Supply-1"
    )
    equipment_list.append(ps)
    
    # Relay Controller
    relay = KTARelayEquipment(
        com_port="/dev/ttyUSB0",  # or "COM3" on Windows
        equipment_name="Relay-1"
    )
    equipment_list.append(relay)
    
    # Connect all equipment
    connected_equipment = []
    for equip in equipment_list:
        print(f"Connecting to {equip.equipment_name}...")
        if equip.connect():
            print(f"  {equip.equipment_name}: Connected")
            connected_equipment.append(equip)
        else:
            print(f"  {equip.equipment_name}: Failed - {equip.get_last_error()}")
    
    # Configure and use equipment
    if connected_equipment:
        print("\\nConfiguring equipment...")
        
        for equip in connected_equipment:
            # Apply default configuration
            equip.apply_config()
            
            # Perform self-test
            print(f"Self-test {equip.equipment_name}: {'PASS' if equip.self_test() else 'FAIL'}")
        
        # Example coordinated measurement
        print("\\nPerforming coordinated measurement...")
        
        # Set power supply output
        if ps in connected_equipment:
            ps.set_voltage(5.0, channel=1)
            ps.set_current_limit(0.5, channel=1)
            ps.enable_output(True)
            print("Power supply: Set to 5V, 0.5A limit, Output ON")
        
        # Control relays
        if relay in connected_equipment:
            relay.set_relay_state(1, True)  # Turn on relay 1
            relay.set_relay_state(2, True)  # Turn on relay 2
            print("Relays: Activated relays 1 and 2")
        
        # Measure voltage
        if dmm in connected_equipment:
            voltage = dmm.measure_voltage_dc()
            if voltage is not None:
                print(f"DMM: Measured {voltage:.3f} V")
        
        # Cleanup
        print("\\nCleaning up...")
        if ps in connected_equipment:
            ps.enable_output(False)
            print("Power supply: Output OFF")
        
        if relay in connected_equipment:
            # Turn off all relays
            for i in range(relay.config['num_relays']):
                relay.set_relay_state(i + 1, False)
            print("Relays: All relays OFF")
    
    # Disconnect all
    print("\\nDisconnecting all equipment...")
    for equip in equipment_list:
        equip.disconnect()


def example_configuration_management():
    """Example: Configuration save/load."""
    print("\\n=== Configuration Management Example ===")
    
    # Create equipment
    dmm = AgilentDMMEquipment("TCPIP::192.168.1.100::INSTR")
    
    # Set custom configuration
    custom_config = {
        'measurement_function': 'resistance',
        'auto_range': False,
        'range': 1000.0,
        'aperture': 0.1,
        'sample_rate': 5.0
    }
    
    dmm.set_config(custom_config)
    print("Applied custom configuration")
    
    # Export configuration
    dmm.export_data("dmm_config.json", "json")
    print("Exported configuration and data")
    
    # Show current config
    current_config = dmm.get_config()
    print("Current configuration:")
    for key, value in current_config.items():
        print(f"  {key}: {value}")


def example_camera_usage():
    """Example: Camera equipment usage."""
    print("\\n=== Camera Equipment Example ===")
    
    try:
        # Create camera equipment
        camera = CameraEquipment(camera_index=0, equipment_name="Lab Camera")
        
        print("Connecting to camera...")
        if camera.connect():
            print("Camera connected successfully!")
            
            # Configure camera
            camera.set_config({
                'trigger_mode': 'software',
                'exposure_time': 10000,  # microseconds
                'gain': 0.0,
                'pixel_format': 'Mono8'
            })
            
            # Capture single image
            print("Capturing image...")
            image_data = camera.capture_image("test_image.jpg")
            if image_data:
                print(f"Image captured: {image_data['width']}x{image_data['height']} pixels")
            
            # Start continuous acquisition
            print("Starting continuous acquisition...")
            camera.start_measurement()
            
            # Capture a few images
            import time
            for i in range(3):
                time.sleep(1)
                img = camera.measure()
                if img:
                    print(f"Frame {i+1}: {img['frame_id']}")
            
            camera.stop_measurement()
            camera.disconnect()
            print("Camera disconnected")
            
        else:
            print(f"Camera connection failed: {camera.get_last_error()}")
    
    except Exception as e:
        print(f"Camera example failed: {str(e)}")
        print("Note: PySpin SDK may not be installed")


def create_example_config():
    """Create an example equipment configuration file."""
    import json
    
    example_config = {
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
        },
        "Relay_Controller": {
            "type": "kta_relay",
            "config": {
                "name": "KTA Relay Box",
                "com_port": "COM3",
                "baud_rate": 9600
            }
        },
        "Camera_Overhead": {
            "type": "camera",
            "config": {
                "name": "Overhead Camera",
                "camera_index": 0
            }
        }
    }
    
    with open("equipment_config.json", "w") as f:
        json.dump(example_config, f, indent=2)
    
    print("Created example_config.json")
    print("This file can be loaded by the Equipment Controller")


def main():
    """Main example runner."""
    print("Universal Equipment Controller - Examples")
    print("=" * 50)
    
    # Create example configuration file
    create_example_config()
    
    # Run examples
    example_single_equipment()
    example_multiple_equipment()
    example_configuration_management()
    example_camera_usage()
    
    print("\\n" + "=" * 50)
    print("To run the main Equipment Controller GUI:")
    print("python equipment_controller.py")
    print("\\nOr call run_main_controller() function")
    
    # Uncomment to launch main GUI
    # run_main_controller()


if __name__ == "__main__":
    main()