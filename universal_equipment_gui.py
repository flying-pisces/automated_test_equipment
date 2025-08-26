"""
Universal Equipment GUI

Provides a standardized GUI interface for all equipment types.
Left panel: Configuration attributes
Right panel: Control methods and status
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import json
from typing import Dict, Any, List, Optional
from base_equipment import BaseEquipment, EquipmentStatus
import threading
import time
from datetime import datetime


class EquipmentConfigPanel(ttk.Frame):
    """Left panel for equipment configuration attributes."""
    
    def __init__(self, parent, equipment: BaseEquipment):
        super().__init__(parent)
        self.equipment = equipment
        self.config_vars = {}
        
        self.setup_ui()
        self.refresh_config()
    
    def setup_ui(self):
        """Setup configuration UI elements."""
        # Title
        title_label = ttk.Label(self, text="Configuration", font=('Arial', 12, 'bold'))
        title_label.pack(pady=5)
        
        # Scrollable frame for config items
        canvas = tk.Canvas(self)
        scrollbar = ttk.Scrollbar(self, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)
        
        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )
        
        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        self.config_frame = scrollable_frame
        
        # Apply button
        apply_frame = ttk.Frame(self)
        apply_frame.pack(side="bottom", fill="x", pady=5)
        
        ttk.Button(apply_frame, text="Apply Config", command=self.apply_config).pack(side="left")
        ttk.Button(apply_frame, text="Reset", command=self.reset_config).pack(side="left", padx=5)
        ttk.Button(apply_frame, text="Load", command=self.load_config).pack(side="left")
        ttk.Button(apply_frame, text="Save", command=self.save_config).pack(side="left", padx=5)
    
    def refresh_config(self):
        """Refresh configuration display."""
        # Clear existing widgets
        for widget in self.config_frame.winfo_children():
            widget.destroy()
        
        self.config_vars.clear()
        
        # Get current config
        config = self.equipment.get_config()
        
        row = 0
        for key, value in config.items():
            self._create_config_widget(key, value, row)
            row += 1
    
    def _create_config_widget(self, key: str, value: Any, row: int):
        """Create appropriate widget for config parameter."""
        # Label
        label = ttk.Label(self.config_frame, text=f"{key}:")
        label.grid(row=row, column=0, sticky="w", padx=5, pady=2)
        
        # Widget based on value type
        if isinstance(value, bool):
            var = tk.BooleanVar(value=value)
            widget = ttk.Checkbutton(self.config_frame, variable=var)
        elif isinstance(value, (int, float)):
            var = tk.StringVar(value=str(value))
            widget = ttk.Entry(self.config_frame, textvariable=var, width=15)
        elif isinstance(value, str):
            var = tk.StringVar(value=value)
            widget = ttk.Entry(self.config_frame, textvariable=var, width=20)
        elif isinstance(value, list):
            var = tk.StringVar(value=str(value))
            widget = ttk.Entry(self.config_frame, textvariable=var, width=25)
        else:
            var = tk.StringVar(value=str(value))
            widget = ttk.Entry(self.config_frame, textvariable=var, width=20)
        
        widget.grid(row=row, column=1, sticky="ew", padx=5, pady=2)
        self.config_frame.columnconfigure(1, weight=1)
        
        self.config_vars[key] = (var, type(value))
    
    def apply_config(self):
        """Apply configuration changes to equipment."""
        try:
            new_config = {}
            
            for key, (var, value_type) in self.config_vars.items():
                raw_value = var.get()
                
                # Convert to appropriate type
                if value_type == bool:
                    new_config[key] = bool(raw_value)
                elif value_type == int:
                    new_config[key] = int(raw_value)
                elif value_type == float:
                    new_config[key] = float(raw_value)
                elif value_type == list:
                    # Simple list parsing
                    new_config[key] = eval(raw_value) if raw_value.startswith('[') else [raw_value]
                else:
                    new_config[key] = raw_value
            
            if self.equipment.set_config(new_config):
                messagebox.showinfo("Success", "Configuration applied successfully")
            else:
                messagebox.showerror("Error", f"Configuration failed: {self.equipment.get_last_error()}")
                
        except Exception as e:
            messagebox.showerror("Error", f"Configuration error: {str(e)}")
    
    def reset_config(self):
        """Reset configuration to defaults."""
        if messagebox.askyesno("Confirm", "Reset to default configuration?"):
            if self.equipment.reset():
                self.refresh_config()
                messagebox.showinfo("Success", "Configuration reset successfully")
            else:
                messagebox.showerror("Error", f"Reset failed: {self.equipment.get_last_error()}")
    
    def load_config(self):
        """Load configuration from file."""
        filename = filedialog.askopenfilename(
            title="Load Configuration",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if filename:
            try:
                with open(filename, 'r') as f:
                    config_data = json.load(f)
                
                if self.equipment.set_config(config_data):
                    self.refresh_config()
                    messagebox.showinfo("Success", "Configuration loaded successfully")
                else:
                    messagebox.showerror("Error", f"Load failed: {self.equipment.get_last_error()}")
                    
            except Exception as e:
                messagebox.showerror("Error", f"Load error: {str(e)}")
    
    def save_config(self):
        """Save current configuration to file."""
        filename = filedialog.asksaveasfilename(
            title="Save Configuration",
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if filename:
            try:
                config = self.equipment.get_config()
                with open(filename, 'w') as f:
                    json.dump(config, f, indent=2)
                
                messagebox.showinfo("Success", "Configuration saved successfully")
                
            except Exception as e:
                messagebox.showerror("Error", f"Save error: {str(e)}")


class EquipmentControlPanel(ttk.Frame):
    """Right panel for equipment control methods and status."""
    
    def __init__(self, parent, equipment: BaseEquipment):
        super().__init__(parent)
        self.equipment = equipment
        self.status_vars = {}
        self.measurement_thread = None
        self.measurement_running = False
        
        self.setup_ui()
        self.update_status()
    
    def setup_ui(self):
        """Setup control UI elements."""
        # Connection Control
        conn_frame = ttk.LabelFrame(self, text="Connection")
        conn_frame.pack(fill="x", padx=5, pady=5)
        
        ttk.Button(conn_frame, text="Connect", command=self.connect_equipment).pack(side="left", padx=2)
        ttk.Button(conn_frame, text="Disconnect", command=self.disconnect_equipment).pack(side="left", padx=2)
        ttk.Button(conn_frame, text="Self Test", command=self.self_test).pack(side="left", padx=2)
        
        self.conn_status_var = tk.StringVar(value="Disconnected")
        ttk.Label(conn_frame, textvariable=self.conn_status_var).pack(side="right")
        
        # Measurement Control
        meas_frame = ttk.LabelFrame(self, text="Measurement")
        meas_frame.pack(fill="x", padx=5, pady=5)
        
        ttk.Button(meas_frame, text="Start", command=self.start_measurement).pack(side="left", padx=2)
        ttk.Button(meas_frame, text="Stop", command=self.stop_measurement).pack(side="left", padx=2)
        ttk.Button(meas_frame, text="Single", command=self.single_measurement).pack(side="left", padx=2)
        ttk.Button(meas_frame, text="Reset", command=self.reset_equipment).pack(side="left", padx=2)
        
        self.meas_status_var = tk.StringVar(value="Stopped")
        ttk.Label(meas_frame, textvariable=self.meas_status_var).pack(side="right")
        
        # Data Control
        data_frame = ttk.LabelFrame(self, text="Data")
        data_frame.pack(fill="x", padx=5, pady=5)
        
        ttk.Button(data_frame, text="Export Data", command=self.export_data).pack(side="left", padx=2)
        ttk.Button(data_frame, text="Clear Data", command=self.clear_data).pack(side="left", padx=2)
        ttk.Button(data_frame, text="View Data", command=self.view_data).pack(side="left", padx=2)
        
        self.data_count_var = tk.StringVar(value="0 points")
        ttk.Label(data_frame, textvariable=self.data_count_var).pack(side="right")
        
        # Status Display\n        status_frame = ttk.LabelFrame(self, text="Status")
        status_frame.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Status text area
        self.status_text = tk.Text(status_frame, height=10, width=40)
        status_scroll = ttk.Scrollbar(status_frame, orient="vertical", command=self.status_text.yview)
        self.status_text.configure(yscrollcommand=status_scroll.set)
        
        self.status_text.pack(side="left", fill="both", expand=True)
        status_scroll.pack(side="right", fill="y")
        
        # Latest measurement display
        latest_frame = ttk.LabelFrame(self, text="Latest Measurement")
        latest_frame.pack(fill="x", padx=5, pady=5)
        
        self.latest_measurement_var = tk.StringVar(value="No data")
        ttk.Label(latest_frame, textvariable=self.latest_measurement_var, wraplength=300).pack(padx=5, pady=5)
    
    def connect_equipment(self):
        """Connect to equipment."""
        try:
            if self.equipment.connect():
                self.log_status("Connected successfully")
                messagebox.showinfo("Success", "Equipment connected successfully")
            else:
                self.log_status(f"Connection failed: {self.equipment.get_last_error()}")
                messagebox.showerror("Error", f"Connection failed: {self.equipment.get_last_error()}")
        except Exception as e:
            self.log_status(f"Connection error: {str(e)}")
            messagebox.showerror("Error", f"Connection error: {str(e)}")
        
        self.update_status()
    
    def disconnect_equipment(self):
        """Disconnect from equipment."""
        try:
            if self.measurement_running:
                self.stop_measurement()
            
            if self.equipment.disconnect():
                self.log_status("Disconnected successfully")
                messagebox.showinfo("Success", "Equipment disconnected")
            else:
                self.log_status(f"Disconnection failed: {self.equipment.get_last_error()}")
        except Exception as e:
            self.log_status(f"Disconnection error: {str(e)}")
        
        self.update_status()
    
    def self_test(self):
        """Perform equipment self-test."""
        try:
            self.log_status("Performing self-test...")
            if self.equipment.self_test():
                self.log_status("Self-test PASSED")
                messagebox.showinfo("Success", "Self-test passed")
            else:
                self.log_status(f"Self-test FAILED: {self.equipment.get_last_error()}")
                messagebox.showwarning("Warning", "Self-test failed")
        except Exception as e:
            self.log_status(f"Self-test error: {str(e)}")
            messagebox.showerror("Error", f"Self-test error: {str(e)}")
    
    def start_measurement(self):
        """Start continuous measurement."""
        try:
            if self.equipment.start_measurement():
                self.measurement_running = True
                self.log_status("Measurement started")
                
                # Start measurement thread
                self.measurement_thread = threading.Thread(target=self._measurement_loop, daemon=True)
                self.measurement_thread.start()
            else:
                self.log_status(f"Start measurement failed: {self.equipment.get_last_error()}")
                messagebox.showerror("Error", f"Start failed: {self.equipment.get_last_error()}")
        except Exception as e:
            self.log_status(f"Start measurement error: {str(e)}")
            messagebox.showerror("Error", f"Start error: {str(e)}")
        
        self.update_status()
    
    def stop_measurement(self):
        """Stop continuous measurement."""
        try:
            self.measurement_running = False
            
            if self.equipment.stop_measurement():
                self.log_status("Measurement stopped")
            else:
                self.log_status(f"Stop measurement failed: {self.equipment.get_last_error()}")
        except Exception as e:
            self.log_status(f"Stop measurement error: {str(e)}")
        
        # Wait for measurement thread to finish
        if self.measurement_thread and self.measurement_thread.is_alive():
            self.measurement_thread.join(timeout=2.0)
        
        self.update_status()
    
    def single_measurement(self):
        """Perform single measurement."""
        try:
            self.log_status("Taking single measurement...")
            result = self.equipment.measure()
            
            if result is not None:
                self.log_status(f"Measurement: {result}")
                self.latest_measurement_var.set(str(result))
            else:
                self.log_status(f"Measurement failed: {self.equipment.get_last_error()}")
                messagebox.showerror("Error", f"Measurement failed: {self.equipment.get_last_error()}")
        except Exception as e:
            self.log_status(f"Measurement error: {str(e)}")
            messagebox.showerror("Error", f"Measurement error: {str(e)}")
        
        self.update_status()
    
    def reset_equipment(self):
        """Reset equipment."""
        if messagebox.askyesno("Confirm", "Reset equipment to default state?"):
            try:
                if self.equipment.reset():
                    self.log_status("Equipment reset successfully")
                    messagebox.showinfo("Success", "Equipment reset")
                else:
                    self.log_status(f"Reset failed: {self.equipment.get_last_error()}")
                    messagebox.showerror("Error", f"Reset failed: {self.equipment.get_last_error()}")
            except Exception as e:
                self.log_status(f"Reset error: {str(e)}")
                messagebox.showerror("Error", f"Reset error: {str(e)}")
            
            self.update_status()
    
    def export_data(self):
        """Export measurement data."""
        data = self.equipment.get_measurement_data()
        if not data:
            messagebox.showinfo("Info", "No data to export")
            return
        
        filename = filedialog.asksaveasfilename(
            title="Export Data",
            defaultextension=".json",
            filetypes=[
                ("JSON files", "*.json"),
                ("CSV files", "*.csv"),
                ("Text files", "*.txt"),
                ("All files", "*.*")
            ]
        )
        
        if filename:
            try:
                format_type = filename.split('.')[-1].lower()
                if self.equipment.export_data(filename, format_type):
                    self.log_status(f"Data exported to {filename}")
                    messagebox.showinfo("Success", f"Data exported to {filename}")
                else:
                    messagebox.showerror("Error", f"Export failed: {self.equipment.get_last_error()}")
            except Exception as e:
                self.log_status(f"Export error: {str(e)}")
                messagebox.showerror("Error", f"Export error: {str(e)}")
    
    def clear_data(self):
        """Clear measurement data."""
        if messagebox.askyesno("Confirm", "Clear all measurement data?"):
            self.equipment.clear_data()
            self.log_status("Measurement data cleared")
            self.update_status()
    
    def view_data(self):
        """View measurement data in popup window."""
        data = self.equipment.get_measurement_data()
        if not data:
            messagebox.showinfo("Info", "No data to view")
            return
        
        # Create data viewer window
        data_window = tk.Toplevel(self)
        data_window.title(f"{self.equipment.equipment_name} - Data Viewer")
        data_window.geometry("600x400")
        
        # Data display
        text_widget = tk.Text(data_window)
        scroll = ttk.Scrollbar(data_window, orient="vertical", command=text_widget.yview)
        text_widget.configure(yscrollcommand=scroll.set)
        
        # Format and display data
        for i, record in enumerate(data[-100:]):  # Show last 100 records
            text_widget.insert(tk.END, f"Record {len(data)-100+i+1}:\n")
            text_widget.insert(tk.END, json.dumps(record, indent=2) + "\n\n")
        
        text_widget.pack(side="left", fill="both", expand=True)
        scroll.pack(side="right", fill="y")
    
    def _measurement_loop(self):
        """Continuous measurement loop (runs in separate thread)."""
        while self.measurement_running:
            try:
                result = self.equipment.measure()
                if result is not None:
                    # Update UI from main thread
                    self.after_idle(lambda: self.latest_measurement_var.set(str(result)))
                    self.after_idle(lambda: self.update_status())
                
                time.sleep(1.0)  # Measurement interval
                
            except Exception as e:
                self.after_idle(lambda: self.log_status(f"Measurement loop error: {str(e)}"))
                break
    
    def log_status(self, message: str):
        """Add message to status log."""
        timestamp = datetime.now().strftime("%H:%M:%S")
        full_message = f"[{timestamp}] {message}\n"
        
        self.status_text.insert(tk.END, full_message)
        self.status_text.see(tk.END)
    
    def update_status(self):
        """Update status displays."""
        status = self.equipment.get_status()
        
        # Connection status
        self.conn_status_var.set(f"{status['status']} ({'connected' if status['connected'] else 'disconnected'})")
        
        # Measurement status
        meas_status = "Running" if status['measuring'] else "Stopped"
        self.meas_status_var.set(meas_status)
        
        # Data count
        self.data_count_var.set(f"{status['data_points']} points")


class UniversalEquipmentGUI:
    """Main GUI window for universal equipment control."""
    
    def __init__(self, equipment: BaseEquipment):
        self.equipment = equipment
        self.root = tk.Tk()
        self.setup_window()
        self.setup_ui()
    
    def setup_window(self):
        """Setup main window."""
        self.root.title(f"Equipment Controller - {self.equipment.equipment_name}")
        self.root.geometry("1000x600")
        self.root.minsize(800, 500)
        
        # Configure grid weights
        self.root.columnconfigure(0, weight=1)
        self.root.columnconfigure(1, weight=1) 
        self.root.rowconfigure(0, weight=1)
    
    def setup_ui(self):
        """Setup UI layout."""
        # Left panel - Configuration
        left_frame = ttk.LabelFrame(self.root, text="Configuration")
        left_frame.grid(row=0, column=0, sticky="nsew", padx=5, pady=5)
        
        self.config_panel = EquipmentConfigPanel(left_frame, self.equipment)
        self.config_panel.pack(fill="both", expand=True)
        
        # Right panel - Control
        right_frame = ttk.LabelFrame(self.root, text="Control & Status")
        right_frame.grid(row=0, column=1, sticky="nsew", padx=5, pady=5)
        
        self.control_panel = EquipmentControlPanel(right_frame, self.equipment)
        self.control_panel.pack(fill="both", expand=True)
        
        # Menu bar
        self.setup_menu()
    
    def setup_menu(self):
        """Setup menu bar."""
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        # File menu
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Export Data", command=self.control_panel.export_data)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit)
        
        # Equipment menu
        equip_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Equipment", menu=equip_menu)
        equip_menu.add_command(label="Connect", command=self.control_panel.connect_equipment)
        equip_menu.add_command(label="Disconnect", command=self.control_panel.disconnect_equipment)
        equip_menu.add_separator()
        equip_menu.add_command(label="Self Test", command=self.control_panel.self_test)
        equip_menu.add_command(label="Reset", command=self.control_panel.reset_equipment)
        
        # Help menu
        help_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Help", menu=help_menu)
        help_menu.add_command(label="About", command=self.show_about)
    
    def show_about(self):
        """Show about dialog."""
        info = self.equipment.get_identification()
        about_text = f"""Universal Equipment Controller
        
Equipment: {self.equipment.equipment_name}
I/O Type: {self.equipment.io_type.value}
Status: {self.equipment.status.value}

Device Information:
Manufacturer: {info.get('manufacturer', 'Unknown')}
Model: {info.get('model', 'Unknown')}
Serial: {info.get('serial_number', 'Unknown')}
Firmware: {info.get('firmware', 'Unknown')}
"""
        messagebox.showinfo("About", about_text)
    
    def run(self):
        """Start GUI main loop."""
        self.root.mainloop()


# Example usage function
def create_equipment_gui(equipment: BaseEquipment) -> UniversalEquipmentGUI:
    """
    Create and return a GUI for the specified equipment.
    
    Args:
        equipment: BaseEquipment instance
        
    Returns:
        UniversalEquipmentGUI instance
    """
    return UniversalEquipmentGUI(equipment)