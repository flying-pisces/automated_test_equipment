"""
Equipment Controller - Main Application

Provides a centralized controller to instantiate and manage all laboratory equipment.
Supports multiple equipment types with unified GUI interface.
"""

import tkinter as tk
from tkinter import ttk, messagebox
import json
import os
from typing import Dict, List, Optional, Type
import importlib.util

# Import base classes
from base_equipment import BaseEquipment, IOType, EquipmentStatus
from universal_equipment_gui import UniversalEquipmentGUI

# Import specific equipment classes
from ethernet_io.agilent_dmm_equipment import AgilentDMMEquipment
from ethernet_io.agilent_power_supply_equipment import AgilentPowerSupplyEquipment
from serial_port.kta_relay_equipment import KTARelayEquipment
from usb_io.camera_equipment import CameraEquipment


class EquipmentFactory:
    """Factory class to create equipment instances based on configuration."""
    
    EQUIPMENT_TYPES = {
        # Ethernet/VISA Equipment
        'agilent_dmm': AgilentDMMEquipment,
        'agilent_power_supply': AgilentPowerSupplyEquipment,
        
        # Serial Equipment
        'kta_relay': KTARelayEquipment,
        
        # USB Equipment
        'camera': CameraEquipment,
    }
    
    @classmethod
    def create_equipment(cls, equipment_type: str, config: Dict) -> Optional[BaseEquipment]:
        """
        Create equipment instance from configuration.
        
        Args:
            equipment_type: Type of equipment to create
            config: Equipment configuration parameters
            
        Returns:
            BaseEquipment instance or None if creation fails
        """
        if equipment_type not in cls.EQUIPMENT_TYPES:
            raise ValueError(f"Unknown equipment type: {equipment_type}")
        
        equipment_class = cls.EQUIPMENT_TYPES[equipment_type]
        
        try:
            # Extract parameters based on equipment type
            if equipment_type == 'agilent_dmm':
                return equipment_class(
                    visa_address=config['visa_address'],
                    equipment_name=config.get('name', 'Agilent DMM')
                )
            
            elif equipment_type == 'agilent_power_supply':
                return equipment_class(
                    visa_address=config['visa_address'],
                    model=config.get('model', 'e3640a'),
                    equipment_name=config.get('name', 'Agilent Power Supply')
                )
            
            elif equipment_type == 'kta_relay':
                return equipment_class(
                    com_port=config['com_port'],
                    baud_rate=config.get('baud_rate', 9600),
                    equipment_name=config.get('name', 'KTA Relay Controller')
                )
            
            elif equipment_type == 'camera':
                return equipment_class(
                    camera_index=config.get('camera_index', 0),
                    equipment_name=config.get('name', 'FLIR Camera')
                )
            
            else:
                return None
                
        except Exception as e:
            print(f"Failed to create {equipment_type}: {str(e)}")
            return None
    
    @classmethod
    def get_available_types(cls) -> List[str]:
        """Get list of available equipment types."""
        return list(cls.EQUIPMENT_TYPES.keys())


class EquipmentManagerGUI:
    """Main GUI for managing multiple equipment instances."""
    
    def __init__(self):
        self.root = tk.Tk()
        self.equipment_instances = {}  # Dict[str, BaseEquipment]
        self.equipment_guis = {}       # Dict[str, UniversalEquipmentGUI]
        self.equipment_configs = {}    # Dict[str, Dict]
        
        self.setup_window()
        self.setup_ui()
        self.load_configurations()
    
    def setup_window(self):
        """Setup main manager window."""
        self.root.title("Laboratory Equipment Controller")
        self.root.geometry("800x600")
        self.root.minsize(600, 400)
    
    def setup_ui(self):
        """Setup UI layout."""
        # Menu bar
        self.setup_menu()
        
        # Main frame
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill="both", expand=True, padx=10, pady=10)
        
        # Equipment list frame
        list_frame = ttk.LabelFrame(main_frame, text="Equipment List")
        list_frame.pack(fill="both", expand=True)
        
        # Equipment tree view
        columns = ('Name', 'Type', 'I/O', 'Status', 'Connected')
        self.equipment_tree = ttk.Treeview(list_frame, columns=columns, show='headings')
        
        for col in columns:
            self.equipment_tree.heading(col, text=col)
            self.equipment_tree.column(col, width=120)
        
        # Scrollbar for tree
        tree_scroll = ttk.Scrollbar(list_frame, orient="vertical", command=self.equipment_tree.yview)
        self.equipment_tree.configure(yscrollcommand=tree_scroll.set)
        
        self.equipment_tree.pack(side="left", fill="both", expand=True)
        tree_scroll.pack(side="right", fill="y")
        
        # Control buttons frame
        button_frame = ttk.Frame(main_frame)
        button_frame.pack(fill="x", pady=10)
        
        ttk.Button(button_frame, text="Add Equipment", command=self.add_equipment_dialog).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Remove Equipment", command=self.remove_equipment).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Open Control Panel", command=self.open_control_panel).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Connect All", command=self.connect_all).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Disconnect All", command=self.disconnect_all).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Refresh", command=self.refresh_equipment_list).pack(side="right", padx=5)
        
        # Status frame
        status_frame = ttk.LabelFrame(main_frame, text="System Status")
        status_frame.pack(fill="x", pady=5)
        
        self.status_text = tk.Text(status_frame, height=6)
        status_scroll = ttk.Scrollbar(status_frame, orient="vertical", command=self.status_text.yview)
        self.status_text.configure(yscrollcommand=status_scroll.set)
        
        self.status_text.pack(side="left", fill="both", expand=True)
        status_scroll.pack(side="right", fill="y")
    
    def setup_menu(self):
        """Setup menu bar."""
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        # File menu
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Load Configuration", command=self.load_configuration_file)
        file_menu.add_command(label="Save Configuration", command=self.save_configuration_file)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit)
        
        # Equipment menu
        equip_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Equipment", menu=equip_menu)
        equip_menu.add_command(label="Add Equipment", command=self.add_equipment_dialog)
        equip_menu.add_command(label="Remove Equipment", command=self.remove_equipment)
        equip_menu.add_separator()
        equip_menu.add_command(label="Connect All", command=self.connect_all)
        equip_menu.add_command(label="Disconnect All", command=self.disconnect_all)
        
        # Help menu
        help_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Help", menu=help_menu)
        help_menu.add_command(label="About", command=self.show_about)
    
    def add_equipment_dialog(self):
        """Show dialog to add new equipment."""
        dialog = EquipmentConfigDialog(self.root, self.add_equipment)
        self.root.wait_window(dialog.dialog)
    
    def add_equipment(self, name: str, equipment_type: str, config: Dict):
        """Add new equipment instance."""
        try:
            # Create equipment instance
            equipment = EquipmentFactory.create_equipment(equipment_type, config)
            if not equipment:
                raise Exception(f"Failed to create equipment of type {equipment_type}")
            
            # Store equipment
            self.equipment_instances[name] = equipment
            self.equipment_configs[name] = {
                'type': equipment_type,
                'config': config
            }
            
            self.log_status(f"Added equipment: {name} ({equipment_type})")
            self.refresh_equipment_list()
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to add equipment: {str(e)}")
            self.log_status(f"Failed to add equipment {name}: {str(e)}")
    
    def remove_equipment(self):
        """Remove selected equipment."""
        selection = self.equipment_tree.selection()
        if not selection:
            messagebox.showwarning("Warning", "Please select equipment to remove")
            return
        
        # Get equipment name from selection
        item = self.equipment_tree.item(selection[0])
        equipment_name = item['values'][0]
        
        if messagebox.askyesno("Confirm", f"Remove equipment '{equipment_name}'?"):
            try:
                # Disconnect and remove
                if equipment_name in self.equipment_instances:
                    equipment = self.equipment_instances[equipment_name]
                    equipment.disconnect()
                    del self.equipment_instances[equipment_name]
                
                if equipment_name in self.equipment_configs:
                    del self.equipment_configs[equipment_name]
                
                # Close GUI if open
                if equipment_name in self.equipment_guis:
                    gui = self.equipment_guis[equipment_name]
                    gui.root.destroy()
                    del self.equipment_guis[equipment_name]
                
                self.log_status(f"Removed equipment: {equipment_name}")
                self.refresh_equipment_list()
                
            except Exception as e:
                messagebox.showerror("Error", f"Failed to remove equipment: {str(e)}")
    
    def open_control_panel(self):
        """Open control panel for selected equipment."""
        selection = self.equipment_tree.selection()
        if not selection:
            messagebox.showwarning("Warning", "Please select equipment to control")
            return
        
        # Get equipment name
        item = self.equipment_tree.item(selection[0])
        equipment_name = item['values'][0]
        
        if equipment_name not in self.equipment_instances:
            messagebox.showerror("Error", "Equipment not found")
            return
        
        # Check if GUI already open
        if equipment_name in self.equipment_guis:
            gui = self.equipment_guis[equipment_name]
            gui.root.lift()  # Bring to front
            return
        
        try:
            # Create new GUI
            equipment = self.equipment_instances[equipment_name]
            gui = UniversalEquipmentGUI(equipment)
            self.equipment_guis[equipment_name] = gui
            
            # Handle GUI window closing
            def on_gui_close():
                if equipment_name in self.equipment_guis:
                    del self.equipment_guis[equipment_name]
                gui.root.destroy()
            
            gui.root.protocol("WM_DELETE_WINDOW", on_gui_close)
            
            # Start GUI in separate window
            gui.root.mainloop()
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to open control panel: {str(e)}")
    
    def connect_all(self):
        """Connect all equipment."""
        connected_count = 0
        failed_count = 0
        
        for name, equipment in self.equipment_instances.items():
            try:
                if equipment.connect():
                    connected_count += 1
                    self.log_status(f"Connected: {name}")
                else:
                    failed_count += 1
                    self.log_status(f"Failed to connect: {name} - {equipment.get_last_error()}")
            except Exception as e:
                failed_count += 1
                self.log_status(f"Connection error for {name}: {str(e)}")
        
        self.refresh_equipment_list()
        self.log_status(f"Connection summary: {connected_count} connected, {failed_count} failed")
        
        if failed_count > 0:
            messagebox.showwarning("Warning", f"Failed to connect {failed_count} equipment(s)")
    
    def disconnect_all(self):
        """Disconnect all equipment."""
        disconnected_count = 0
        
        for name, equipment in self.equipment_instances.items():
            try:
                equipment.disconnect()
                disconnected_count += 1
                self.log_status(f"Disconnected: {name}")
            except Exception as e:
                self.log_status(f"Disconnection error for {name}: {str(e)}")
        
        self.refresh_equipment_list()
        self.log_status(f"Disconnected {disconnected_count} equipment(s)")
    
    def refresh_equipment_list(self):
        """Refresh equipment list display."""
        # Clear existing items
        for item in self.equipment_tree.get_children():
            self.equipment_tree.delete(item)
        
        # Add equipment items
        for name, equipment in self.equipment_instances.items():
            status = equipment.get_status()
            values = (
                name,
                self.equipment_configs[name]['type'],
                equipment.io_type.value,
                status['status'],
                'Yes' if status['connected'] else 'No'
            )
            self.equipment_tree.insert('', 'end', values=values)
    
    def log_status(self, message: str):
        """Add message to status log."""
        from datetime import datetime
        timestamp = datetime.now().strftime("%H:%M:%S")
        full_message = f"[{timestamp}] {message}\n"
        
        self.status_text.insert(tk.END, full_message)
        self.status_text.see(tk.END)
    
    def load_configurations(self):
        """Load equipment configurations from file."""
        config_file = "equipment_config.json"
        if os.path.exists(config_file):
            try:
                with open(config_file, 'r') as f:
                    configs = json.load(f)
                
                for name, config_data in configs.items():
                    self.add_equipment(name, config_data['type'], config_data['config'])
                
                self.log_status(f"Loaded {len(configs)} equipment configurations")
                
            except Exception as e:
                self.log_status(f"Failed to load configurations: {str(e)}")
    
    def load_configuration_file(self):
        """Load configuration from selected file."""
        from tkinter import filedialog
        
        filename = filedialog.askopenfilename(
            title="Load Equipment Configuration",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if filename:
            try:
                with open(filename, 'r') as f:
                    configs = json.load(f)
                
                # Clear existing equipment
                self.equipment_instances.clear()
                self.equipment_configs.clear()
                
                # Load new configurations
                for name, config_data in configs.items():
                    self.add_equipment(name, config_data['type'], config_data['config'])
                
                self.log_status(f"Loaded configuration from {filename}")
                messagebox.showinfo("Success", f"Loaded {len(configs)} equipment configurations")
                
            except Exception as e:
                messagebox.showerror("Error", f"Failed to load configuration: {str(e)}")
    
    def save_configuration_file(self):
        """Save current configuration to file."""
        from tkinter import filedialog
        
        if not self.equipment_configs:
            messagebox.showinfo("Info", "No equipment to save")
            return
        
        filename = filedialog.asksaveasfilename(
            title="Save Equipment Configuration",
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if filename:
            try:
                with open(filename, 'w') as f:
                    json.dump(self.equipment_configs, f, indent=2)
                
                self.log_status(f"Saved configuration to {filename}")
                messagebox.showinfo("Success", "Configuration saved successfully")
                
            except Exception as e:
                messagebox.showerror("Error", f"Failed to save configuration: {str(e)}")
    
    def show_about(self):
        """Show about dialog."""
        about_text = """Laboratory Equipment Controller
        
A unified interface for managing multiple laboratory instruments.

Features:
• Universal equipment interface
• Standardized GUI controls
• Configuration management
• Data export capabilities
• Multi-equipment coordination

Supported Equipment Types:
• Agilent/Keysight DMMs
• Agilent/Keysight Power Supplies  
• KTA Relay Controllers
• FLIR/Point Grey Cameras
• And more...
"""
        messagebox.showinfo("About", about_text)
    
    def run(self):
        """Start main application."""
        self.root.mainloop()


class EquipmentConfigDialog:
    """Dialog for configuring new equipment."""
    
    def __init__(self, parent, callback):
        self.callback = callback
        self.dialog = tk.Toplevel(parent)
        self.dialog.title("Add Equipment")
        self.dialog.geometry("400x500")
        self.dialog.transient(parent)
        self.dialog.grab_set()
        
        self.setup_ui()
    
    def setup_ui(self):
        """Setup dialog UI."""
        # Equipment name
        ttk.Label(self.dialog, text="Equipment Name:").pack(pady=5)
        self.name_var = tk.StringVar()
        ttk.Entry(self.dialog, textvariable=self.name_var, width=40).pack(pady=5)
        
        # Equipment type
        ttk.Label(self.dialog, text="Equipment Type:").pack(pady=5)
        self.type_var = tk.StringVar()
        type_combo = ttk.Combobox(self.dialog, textvariable=self.type_var, 
                                values=EquipmentFactory.get_available_types(), state="readonly")
        type_combo.pack(pady=5)
        type_combo.bind('<<ComboboxSelected>>', self.on_type_changed)
        
        # Configuration frame
        self.config_frame = ttk.LabelFrame(self.dialog, text="Configuration")
        self.config_frame.pack(fill="both", expand=True, padx=10, pady=10)
        
        self.config_widgets = {}
        
        # Buttons
        button_frame = ttk.Frame(self.dialog)
        button_frame.pack(fill="x", padx=10, pady=10)
        
        ttk.Button(button_frame, text="OK", command=self.ok_clicked).pack(side="right", padx=5)
        ttk.Button(button_frame, text="Cancel", command=self.dialog.destroy).pack(side="right")
    
    def on_type_changed(self, event=None):
        """Handle equipment type selection change."""
        # Clear existing config widgets
        for widget in self.config_frame.winfo_children():
            widget.destroy()
        self.config_widgets.clear()
        
        equipment_type = self.type_var.get()
        
        # Create config widgets based on equipment type
        if equipment_type in ['agilent_dmm', 'agilent_power_supply']:
            self._create_visa_config()
            if equipment_type == 'agilent_power_supply':
                self._create_model_config()
        
        elif equipment_type == 'kta_relay':
            self._create_serial_config()
        
        elif equipment_type == 'camera':
            self._create_camera_config()
    
    def _create_visa_config(self):
        """Create VISA configuration widgets."""
        ttk.Label(self.config_frame, text="VISA Address:").grid(row=0, column=0, sticky="w", padx=5, pady=2)
        visa_var = tk.StringVar(value="TCPIP::192.168.1.100::INSTR")
        ttk.Entry(self.config_frame, textvariable=visa_var, width=30).grid(row=0, column=1, padx=5, pady=2)
        self.config_widgets['visa_address'] = visa_var
    
    def _create_model_config(self):
        """Create model selection for power supplies."""
        ttk.Label(self.config_frame, text="Model:").grid(row=1, column=0, sticky="w", padx=5, pady=2)
        model_var = tk.StringVar(value="e3640a")
        ttk.Combobox(self.config_frame, textvariable=model_var, 
                    values=["e3640a", "e3646a", "e3648a"], state="readonly").grid(row=1, column=1, padx=5, pady=2)
        self.config_widgets['model'] = model_var
    
    def _create_serial_config(self):
        """Create serial configuration widgets."""
        ttk.Label(self.config_frame, text="COM Port:").grid(row=0, column=0, sticky="w", padx=5, pady=2)
        com_var = tk.StringVar(value="COM3")
        ttk.Entry(self.config_frame, textvariable=com_var, width=15).grid(row=0, column=1, padx=5, pady=2)
        self.config_widgets['com_port'] = com_var
        
        ttk.Label(self.config_frame, text="Baud Rate:").grid(row=1, column=0, sticky="w", padx=5, pady=2)
        baud_var = tk.StringVar(value="9600")
        ttk.Combobox(self.config_frame, textvariable=baud_var, 
                    values=["9600", "19200", "38400", "57600", "115200"]).grid(row=1, column=1, padx=5, pady=2)
        self.config_widgets['baud_rate'] = baud_var
    
    def _create_camera_config(self):
        """Create camera configuration widgets."""
        ttk.Label(self.config_frame, text="Camera Index:").grid(row=0, column=0, sticky="w", padx=5, pady=2)
        index_var = tk.StringVar(value="0")
        ttk.Spinbox(self.config_frame, textvariable=index_var, from_=0, to=10, width=15).grid(row=0, column=1, padx=5, pady=2)
        self.config_widgets['camera_index'] = index_var
    
    def ok_clicked(self):
        """Handle OK button click."""
        try:
            name = self.name_var.get().strip()
            equipment_type = self.type_var.get()
            
            if not name:
                messagebox.showerror("Error", "Please enter equipment name")
                return
            
            if not equipment_type:
                messagebox.showerror("Error", "Please select equipment type")
                return
            
            # Build configuration dictionary
            config = {'name': name}
            for key, var in self.config_widgets.items():
                value = var.get()
                # Convert numeric values
                if key in ['baud_rate', 'camera_index']:
                    value = int(value)
                config[key] = value
            
            # Call callback to add equipment
            self.callback(name, equipment_type, config)
            self.dialog.destroy()
            
        except Exception as e:
            messagebox.showerror("Error", f"Configuration error: {str(e)}")


# Main application entry point
def main():
    """Main application entry point."""
    app = EquipmentManagerGUI()
    app.run()


if __name__ == "__main__":
    main()