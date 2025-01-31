import sys
import csv
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton, QLabel, QLineEdit
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
from PyQt5.QtCore import QThread, pyqtSignal
import serial
import time
import struct

def compute_crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x8005
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc

class SerialReader(QThread):
    data_received = pyqtSignal(str, int, int, bool)

    def __init__(self, port, baud_rate):
        super().__init__()
        self.port = port
        self.baud_rate = baud_rate
        self.ser = None
        self.running = True

    def run(self):
        try:
            self.ser = serial.Serial(self.port, self.baud_rate)
            while self.running:
                if self.ser.in_waiting > 0:
                    data = self.ser.read_until(b'\n')
                    received_crc = struct.unpack('<H', self.ser.read(2))[0]

                    computed_crc = compute_crc16(data)

                    valid_crc = received_crc == computed_crc
                    self.data_received.emit(data.decode('utf-8').strip(), received_crc, computed_crc, valid_crc)
                time.sleep(0.1)
        except serial.SerialException as e:
            self.data_received.emit(f"Error: {e}", 0, 0, False)

    def stop(self):
        self.running = False
        if self.ser:
            self.ser.close()

    def send(self, data):
        if self.ser and self.ser.is_open:
            crc = compute_crc16(data.encode('utf-8'))
            message = f"{data},{crc}\n"
            self.ser.write(message.encode('utf-8'))

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()

        self.serial_port = "COM6"
        self.baud_rate = 115200

        self.data_buffer = {"input": [], "output": [], "fan": [], "hum": []}
        self.max_points = 100

        self.initUI()

        self.serial_thread = SerialReader(self.serial_port, self.baud_rate)
        self.serial_thread.data_received.connect(self.handle_serial_data)
        self.serial_thread.start()

        # Initialize CSV logging
        self.csv_file = "data_log.csv"
        with open(self.csv_file, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(["Input", "Output", "Fan State", "Hum State", "Received CRC", "Computed CRC", "Valid CRC"])

    def initUI(self):
        self.setWindowTitle("Serial Data Plotter")

        self.figure = Figure()
        self.canvas = FigureCanvas(self.figure)

        self.axes_combined = self.figure.add_subplot(311)
        self.axes_combined.set_ylabel("Input & Output")
        self.axes_fan = self.figure.add_subplot(312)
        self.axes_fan.set_ylabel("Fan State")
        self.axes_hum = self.figure.add_subplot(313)
        self.axes_hum.set_ylabel("Hum State")

        # GUI elements
        self.send_button = QPushButton("Send")
        self.send_button.clicked.connect(self.send_data)
        self.value_entry = QLineEdit()
        self.data_label = QLabel("Received Data:")
        self.crc_label = QLabel("CRC Info: None")

        # Layout
        layout = QVBoxLayout()
        layout.addWidget(self.canvas)
        layout.addWidget(self.value_entry)
        layout.addWidget(self.send_button)
        layout.addWidget(self.data_label)
        layout.addWidget(self.crc_label)
        self.setLayout(layout)

    def handle_serial_data(self, data, received_crc, computed_crc, valid_crc):
        self.data_label.setText(f"Received Data: {data}")
        if valid_crc:
            self.crc_label.setText(
                f"CRC Info: Received CRC = {received_crc}, Computed CRC = {computed_crc} (Valid)"
            )
            self.update_plot(data)
            self.log_to_csv(data, received_crc, computed_crc, valid_crc)
        else:
            self.crc_label.setText(
                f"CRC Info: Received CRC = {received_crc}, Computed CRC = {computed_crc} (Invalid)"
            )

    def update_plot(self, data):
        try:
            input_val, output_val, fan_state, hum_state = map(float, data.split(","))
            self.data_buffer["input"].append(input_val)
            self.data_buffer["output"].append(output_val)
            self.data_buffer["fan"].append(fan_state)
            self.data_buffer["hum"].append(hum_state)

            for key in self.data_buffer:
                if len(self.data_buffer[key]) > self.max_points:
                    self.data_buffer[key] = self.data_buffer[key][-self.max_points:]

            self.axes_combined.clear()
            self.axes_fan.clear()
            self.axes_hum.clear()

            self.axes_combined.plot(self.data_buffer["input"], 'r-', label="Target value")
            self.axes_combined.plot(self.data_buffer["output"], 'g-', label="Current value")
            self.axes_combined.legend(loc="upper right")

            self.axes_fan.plot(self.data_buffer["fan"], 'b-')
            self.axes_fan.set_ylabel("Fan State")

            self.axes_hum.plot(self.data_buffer["hum"], 'k-')
            self.axes_hum.set_ylabel("Hum State")

            self.canvas.draw()

        except ValueError as e:
            print(f"Error parsing data: {e}")

    def log_to_csv(self, data, received_crc, computed_crc, valid_crc):
        try:
            input_val, output_val, fan_state, hum_state = map(float, data.split(","))
            with open(self.csv_file, mode='a', newline='') as file:
                writer = csv.writer(file)
                writer.writerow([input_val, output_val, fan_state, hum_state, received_crc, computed_crc, valid_crc])
        except ValueError as e:
            print(f"Error logging data: {e}")

    def send_data(self):
        value_x = self.value_entry.text()
        self.serial_thread.send(value_x.strip())

    def closeEvent(self, event):
        self.serial_thread.stop()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
