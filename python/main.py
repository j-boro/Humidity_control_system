import sys
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton, QLabel, QLineEdit
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
from PyQt5.QtCore import QThread, pyqtSignal
import serial
import time

class SerialReader(QThread):
    data_received = pyqtSignal(str)

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
                    data = self.ser.readline().decode('utf-8').strip()
                    self.data_received.emit(data)
                time.sleep(0.1)
        except serial.SerialException as e:
            self.data_received.emit(f"Error: {e}")

    def stop(self):
        self.running = False
        if self.ser:
            self.ser.close()

    def send(self, data):
        if self.ser and self.ser.is_open:
            self.ser.write(data.encode('utf-8'))

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()

        self.serial_port = "COM3"
        self.baud_rate = 115200

        self.data_buffer = {"input": [], "output": [], "fan": [], "hum": []}
        self.max_points = 100

        self.initUI()

        self.serial_thread = SerialReader(self.serial_port, self.baud_rate)
        self.serial_thread.data_received.connect(self.handle_serial_data)
        self.serial_thread.start()

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

        # Layout
        layout = QVBoxLayout()
        layout.addWidget(self.canvas)
        layout.addWidget(self.value_entry)
        layout.addWidget(self.send_button)
        layout.addWidget(self.data_label)
        self.setLayout(layout)

    def handle_serial_data(self, data):
        self.data_label.setText(f"Received Data: {data}")
        self.update_plot(data)

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

    def send_data(self):
        value_x = self.value_entry.text()
        self.serial_thread.send(value_x.strip() + "\n")

    def closeEvent(self, event):
        self.serial_thread.stop()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
