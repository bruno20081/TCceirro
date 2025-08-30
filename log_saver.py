import serial
import serial.tools.list_ports
from datetime import datetime
import time
import threading
import tkinter as tk
from tkinter import scrolledtext

def find_arduino_port():
    """Tenta encontrar a porta serial do Arduino automaticamente."""
    ports = serial.tools.list_ports.comports()
    for p in ports:
        if 'Arduino' in p.description or 'USB-SERIAL' in p.description:
            return p.device
    return None

def update_gui(log_text_widget, log_entry):
    log_text_widget.config(state=tk.NORMAL)
    log_text_widget.insert(tk.END, log_entry)
    log_text_widget.see(tk.END)
    log_text_widget.config(state=tk.DISABLED)

def serial_thread(log_text_widget):
    port = find_arduino_port()
    if not port:
        log_entry = "Arduino nao encontrado. Por favor, especifique a porta serial manualmente.\n"
        update_gui(log_text_widget, log_entry)
        return
    ser = None
    try:
        ser = serial.Serial(port, 9600, timeout=1)
        update_gui(log_text_widget, f"Conectado ao Arduino na porta: {port}\n")
        time.sleep(2)
        with open("log_acesso.txt", "a") as log_file:
            update_gui(log_text_widget, "Escutando dados do Arduino...\n")
            while True:
                line = ser.readline().decode('utf-8').strip()
                if line.startswith("LOG,"):
                    parts = line.split(',')
                    uid = parts[1]
                    owner = parts[2]
                    status = parts[3]
                    timestamp = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
                    log_entry = f"{timestamp} - UID: {uid}, Dono: {owner}, Status: {status}\n"
                    update_gui(log_text_widget, log_entry)
                    log_file.write(log_entry)
                    log_file.flush()
    except serial.SerialException as e:
        update_gui(log_text_widget, f"Erro de porta serial: {e}\n")
    except Exception as e:
        update_gui(log_text_widget, f"Erro: {e}\n")
    finally:
        if ser and ser.is_open:
            ser.close()
            update_gui(log_text_widget, "Porta serial fechada.\n")

def main():
    root = tk.Tk()
    root.title("Log de Acesso RFID - Arduino")
    log_text = scrolledtext.ScrolledText(root, width=80, height=20, state=tk.DISABLED, font=("Consolas", 10))
    log_text.pack(padx=10, pady=10)
    t = threading.Thread(target=serial_thread, args=(log_text,), daemon=True)
    t.start()
    root.mainloop()

if __name__ == "__main__":
    main()
