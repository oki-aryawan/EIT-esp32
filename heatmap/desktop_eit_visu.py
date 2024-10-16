import os
import sys
import numpy as np
import matplotlib

matplotlib.use('Agg')  # Use Agg backend
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from scipy import interpolate
import serial
import time
import ast
import tkinter as tk
from tkinter import ttk, filedialog
from serial.tools import list_ports


def resource_path(relative_path):
    """ Get absolute path to resource, works for dev and for PyInstaller """
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)


def get_available_ports():
    return [port.device for port in list_ports.comports()]


class EITApplication:
    def __init__(self, master):
        self.master = master
        master.title("EIT Plot Application")
        master.geometry("800x700")  # Increased height to accommodate new buttons

        self.com_frame = ttk.Frame(master)
        self.com_frame.pack(pady=10)

        self.label = ttk.Label(self.com_frame, text="Select COM Port:")
        self.label.pack(side=tk.LEFT, padx=5)

        self.com_port_var = tk.StringVar()
        self.com_port_combo = ttk.Combobox(self.com_frame, textvariable=self.com_port_var)
        self.com_port_combo['values'] = get_available_ports()
        self.com_port_combo.pack(side=tk.LEFT, padx=5)

        self.start_button = ttk.Button(self.com_frame, text="Start", command=self.start_application)
        self.start_button.pack(side=tk.LEFT, padx=5)

        self.save_image_button = ttk.Button(self.com_frame, text="Save Image", command=self.save_image)
        self.save_image_button.pack(side=tk.LEFT, padx=5)
        self.save_image_button.config(state='disabled')  # Initially disabled

        self.save_data_button = ttk.Button(self.com_frame, text="Save Data", command=self.save_data)
        self.save_data_button.pack(side=tk.LEFT, padx=5)
        self.save_data_button.config(state='disabled')  # Initially disabled

        self.status_label = ttk.Label(master, text="")
        self.status_label.pack(pady=5)

        self.fig, self.ax = plt.subplots(figsize=(8, 6))
        self.canvas = FigureCanvasTkAgg(self.fig, master=master)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.pack(fill=tk.BOTH, expand=True)

        self.im = self.ax.imshow(np.zeros((5, 5)), cmap='jet', norm=LogNorm(vmin=1, vmax=1000),
                                 extent=[0, 5, 5, 0], aspect='auto')
        self.ax.set_title('Resistivity Profile (5 Layers)', fontsize=16)
        self.ax.set_xlabel('Distance', fontsize=12)
        self.ax.set_ylabel('Depth', fontsize=12)
        self.ax.set_xticks([])
        self.ax.set_yticks(np.arange(5))
        self.ax.set_yticklabels([f'a={i + 2}' for i in range(5)])

        self.cbar = self.fig.colorbar(self.im)
        self.cbar.set_label('Resistivity in ohm·m', rotation=270, labelpad=15)

        self.ser = None
        self.running = False
        self.last_data = None
        self.all_data = []  # Store all received data

    def start_application(self):
        selected_port = self.com_port_var.get()

        try:
            self.ser = serial.Serial(selected_port, 115200, timeout=1)
            time.sleep(2)  # Wait for the connection to settle
            self.status_label.config(text="Connected. Receiving data...")
            self.running = True
            self.save_image_button.config(state='normal')  # Enable save image button
            self.save_data_button.config(state='normal')  # Enable save data button
            self.master.after(100, self.update_plot)
        except serial.SerialException as e:
            self.status_label.config(text=f"Error: {str(e)}")

    def update_plot(self):
        if not self.running:
            return

        if self.ser and self.ser.in_waiting:
            line = self.ser.readline().decode('utf-8').strip()
            try:
                data = ast.literal_eval(line)
                if isinstance(data, list) and len(data) == 15:
                    self.last_data = data
                    self.all_data.append(data)  # Store the new data
                    self.plot_data(data)
                else:
                    print(f"Received incomplete or invalid data: {line}")
            except (ValueError, SyntaxError) as e:
                print(f"Error parsing data: {line}")
                print(f"Error details: {str(e)}")

        self.master.after(100, self.update_plot)

    def plot_data(self, all_data):
        wenner_data = [
            all_data[:5],
            all_data[5:9],
            all_data[9:12],
            all_data[12:14],
            all_data[14:]
        ]

        plot_data = np.full((5, 5), np.nan)
        for i, row in enumerate(wenner_data):
            start = (5 - len(row)) // 2
            plot_data[i, start:start + len(row)] = row

        mask = ~np.isnan(plot_data)
        y, x = np.mgrid[0:5, 0:5]
        x_upsampled = np.linspace(0, 4, 50)
        y_upsampled = np.linspace(0, 4, 50)
        xx, yy = np.meshgrid(x_upsampled, y_upsampled)

        points = np.column_stack((y[mask].ravel(), x[mask].ravel()))
        values = plot_data[mask].ravel()
        grid_z = interpolate.griddata(points, values, (yy, xx), method='cubic', fill_value=np.nan)

        self.im.set_array(grid_z)
        self.im.set_norm(LogNorm(vmin=np.min(all_data), vmax=np.max(all_data)))
        self.canvas.draw()

    def save_image(self):
        if self.last_data is None:
            self.status_label.config(text="No data to save.")
            return

        file_path = filedialog.asksaveasfilename(defaultextension=".png",
                                                 filetypes=[("PNG files", "*.png"),
                                                            ("All files", "*.*")])
        if file_path:
            self.fig.savefig(file_path)
            self.status_label.config(text=f"Image saved to {file_path}")

    def save_data(self):
        if not self.all_data:
            self.status_label.config(text="No data to save.")
            return

        file_path = filedialog.asksaveasfilename(defaultextension=".txt",
                                                 filetypes=[("Text files", "*.txt"),
                                                            ("All files", "*.*")])
        if file_path:
            with open(file_path, 'w') as f:
                for data in self.all_data:
                    f.write(f"{data}\n")
            self.status_label.config(text=f"Data saved to {file_path}")

    def on_closing(self):
        self.running = False
        if self.ser:
            self.ser.close()
        self.master.destroy()


def main():
    root = tk.Tk()
    app = EITApplication(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()


if __name__ == "__main__":
    main()
