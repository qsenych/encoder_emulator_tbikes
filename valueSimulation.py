import tkinter as tk
from tkinter import messagebox
import serial
import glob
import threading

# Automatically detect serial port
def find_serial_port():
    ports = glob.glob('/dev/tty.usbmodem*')
    return ports[0] if ports else None

serial_port = find_serial_port()
ser = None
if serial_port:
    try:
        ser = serial.Serial(serial_port, 9600, timeout=1)
    except serial.SerialException as e:
        print(f"Serial connection error: {e}")
else:
    print("No serial port found.")

def send_data(value):
    """Send the slider value through the serial port."""
    if not ser or not ser.is_open:
        messagebox.showerror("Error", "Serial port not connected.")
        return

    try:
        if 0 <= value <= 4055:
            ser.write(f"{value}\n".encode())
            status_label.config(text=f"Sent: {value}", fg="green")
        else:
            raise ValueError("Out of range")
    except ValueError:
        status_label.config(text="Invalid value. Ensure it is between 0 and 4055.", fg="red")

def monitor_serial():
    """Continuously read from the serial port and display received data."""
    while True:
        if ser and ser.is_open:
            try:
                data = ser.readline().decode().strip()
                if data:
                    monitor_text.insert(tk.END, f"{data}\n")
                    monitor_text.see(tk.END)
            except Exception as e:
                print(f"Error reading serial data: {e}")
                break

def close_serial():
    """Close the serial connection when the app exits."""
    if ser and ser.is_open:
        ser.close()
    root.destroy()

# Create the main tkinter window
root = tk.Tk()
root.title("Serial Data Sender")
root.geometry("400x300")
root.protocol("WM_DELETE_WINDOW", close_serial)

# Create and place widgets
slider_label = tk.Label(root, text="Select value (0-4055):")
slider_label.pack(pady=10)

slider = tk.Scale(root, from_=0, to=4055, orient="horizontal", command=lambda v: send_data(int(v)))
slider.pack(pady=5, fill="x")

status_label = tk.Label(root, text="", fg="blue")
status_label.pack(pady=10)

monitor_label = tk.Label(root, text="Monitor (Received Data):")
monitor_label.pack(pady=10)

monitor_text = tk.Text(root, height=10, state="normal")
monitor_text.pack(pady=5, fill="both", expand=True)

# Start the serial monitoring in a separate thread
if ser:
    threading.Thread(target=monitor_serial, daemon=True).start()

# Run the tkinter event loop
root.mainloop()
