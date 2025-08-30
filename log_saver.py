import serial
import serial.tools.list_ports
from datetime import datetime
import time

def find_arduino_port():
    """Tenta encontrar a porta serial do Arduino automaticamente."""
    ports = serial.tools.list_ports.comports()
    for p in ports:
        if 'Arduino' in p.description or 'USB-SERIAL' in p.description:
            return p.device
    return None

def main():
    """Lê os dados do Arduino e salva em um arquivo de log."""
    port = find_arduino_port()
    if not port:
        print("Arduino nao encontrado. Por favor, especifique a porta serial manualmente.")
        # Exemplo para Windows: port = 'COM3'
        # Exemplo para macOS/Linux: port = '/dev/tty.usbmodem14201'
        return

    ser = None
    try:
        ser = serial.Serial(port, 9600, timeout=1)
        print(f"Conectado ao Arduino na porta: {port}")
        time.sleep(2) # Espera a conexao ser estabelecida

        with open("log_acesso.txt", "a") as log_file:
            print("Escutando dados do Arduino...")
            while True:
                line = ser.readline().decode('utf-8').strip()
                if line.startswith("LOG,"):
                    parts = line.split(',')
                    uid = parts[1]
                    owner = parts[2]
                    status = parts[3]
                    timestamp = datetime.now().strftime("%d/%m/%Y %H:%M:%S")

                    log_entry = f"{timestamp} - UID: {uid}, Dono: {owner}, Status: {status}\n"
                    
                    print(log_entry.strip())
                    log_file.write(log_entry)
                    log_file.flush() # Salva a linha imediatamente no arquivo

    except serial.SerialException as e:
        print(f"Erro de porta serial: {e}")
    except KeyboardInterrupt:
        print("\nPrograma encerrado pelo usuario.")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Porta serial fechada.")

if __name__ == "__main__":
    main()
