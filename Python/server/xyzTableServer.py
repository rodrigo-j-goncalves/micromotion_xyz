import socket
import serial
import select
import argparse
import sys
import os
import logging
from logging.handlers import TimedRotatingFileHandler
from colorama import init, Fore, Style
import serial.tools.list_ports
from datetime import datetime
from dotenv import load_dotenv

# Inicializar colorama
init(autoreset=True)

# Cargar .env
load_dotenv()

# Leer valores por defecto desde .env o usar hardcoded si faltan
DEFAULTS = {
    'serial_port': os.getenv("SERIAL_PORT", "COM7"),
    'baudrate': int(os.getenv("BAUDRATE", 115200)),
    'host': os.getenv("HOST", "0.0.0.0"),
    'port': int(os.getenv("PORT", 5000)),
}

LOG_DIR = "logs"

def setup_logger():
    os.makedirs(LOG_DIR, exist_ok=True)
    log_filename = os.path.join(LOG_DIR, "xyzTableServer.log")

    logger = logging.getLogger("xyzTableServer")
    logger.setLevel(logging.DEBUG)

    handler = TimedRotatingFileHandler(log_filename, when="midnight", backupCount=7, encoding='utf-8')
    handler.setFormatter(logging.Formatter("[%(asctime)s] [%(levelname)s] %(message)s"))
    logger.addHandler(handler)

    console_handler = logging.StreamHandler()
    console_handler.setFormatter(logging.Formatter("[%(levelname)s] %(message)s"))
    logger.addHandler(console_handler)

    return logger

logger = setup_logger()

def print_banner(args, used_defaults):
    print(Fore.CYAN + Style.BRIGHT + "\n=== xyzTableServer ===")
    print(Fore.YELLOW + "TCP ↔ Serial bridge for Arduino Mega")
    print(Fore.BLUE + f"Started at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
    print(Fore.GREEN + "Running with the following parameters:")
    print(Fore.GREEN + f"  Serial Port: {Fore.WHITE}{args.serial_port}")
    print(Fore.GREEN + f"  Baudrate:    {Fore.WHITE}{args.baudrate}")
    print(Fore.GREEN + f"  TCP Host:    {Fore.WHITE}{args.host}")
    print(Fore.GREEN + f"  TCP Port:    {Fore.WHITE}{args.port}")

    if used_defaults:
        print(Fore.MAGENTA + "\nNo arguments were provided. Default values were used.")
        print(Fore.BLUE + "If you want to customize the parameters, use:")
        print(Fore.WHITE + f"  python {os.path.basename(sys.argv[0])} --help\n")

def list_serial_ports():
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

def parse_args():
    parser = argparse.ArgumentParser(
        prog=os.path.basename(sys.argv[0]),
        description='xyzTableServer: TCP to Serial bridge for Arduino Mega communication.',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('--serial-port', help='Serial port (e.g., COM3 or /dev/ttyUSB0)')
    parser.add_argument('--baudrate', type=int, help='Baudrate for serial communication')
    parser.add_argument('--host', help='TCP server host')
    parser.add_argument('--port', type=int, help='TCP server port')

    if len(sys.argv) == 1:
        args = parser.parse_args([])
        args.serial_port = DEFAULTS['serial_port']
        args.baudrate = DEFAULTS['baudrate']
        args.host = DEFAULTS['host']
        args.port = DEFAULTS['port']
        used_defaults = True
    else:
        args = parser.parse_args()
        args.serial_port = args.serial_port or DEFAULTS['serial_port']
        args.baudrate = args.baudrate or DEFAULTS['baudrate']
        args.host = args.host or DEFAULTS['host']
        args.port = args.port or DEFAULTS['port']
        used_defaults = False

    return args, used_defaults, parser

def main():
    args, used_defaults, parser = parse_args()

    # Mostrar banner y ayuda antes de conectar
    print_banner(args, used_defaults)
    if used_defaults:
        parser.print_help()

    # Conectar al puerto serie
    try:
        ser = serial.Serial(args.serial_port, args.baudrate, timeout=0)
        logger.info(f"Connected to Arduino on {args.serial_port} at {args.baudrate}bps")
    except serial.SerialException as e:
        if isinstance(e.__cause__, PermissionError):
            logger.error(f"Port '{args.serial_port}' is busy or access is denied.")
            print(Fore.RED + f"[ERROR] Port '{args.serial_port}' is busy or access is denied.")
        else:
            logger.error(f"Could not open serial port '{args.serial_port}': {e}")
            print(Fore.RED + f"[ERROR] Could not open serial port '{args.serial_port}': {e}")

        print(Fore.YELLOW + "\nAvailable serial ports:")
        ports = list_serial_ports()
        if ports:
            for port in ports:
                print(Fore.CYAN + f"  - {port}")
        else:
            print(Fore.RED + "  No serial ports found.")
            print(Fore.BLUE + "  Make sure your Arduino is connected and drivers are installed.\n")
        sys.exit(1)

    # Iniciar servidor TCP
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((args.host, args.port))
    server.listen(1)
    logger.info(f"TCP server listening on {args.host}:{args.port}")

    while True:
        logger.info("Waiting for a client...")
        client_sock, addr = server.accept()
        logger.info(f"Client connected from {addr}")
        print(Fore.CYAN + f"[INFO] Client connected from {addr}")
        client_sock.setblocking(False)

        try:
            while True:
                readable, _, _ = select.select([client_sock], [], [], 0.01)
                if readable:
                    data = client_sock.recv(1024)
                    if not data:
                        logger.info("Client disconnected")
                        break
                    ser.write(data)
                    logger.debug(f"[TCP->SERIAL] {data!r}")

                if ser.in_waiting:
                    data = ser.read(ser.in_waiting)
                    client_sock.sendall(data)
                    logger.debug(f"[SERIAL->TCP] {data!r}")

        except Exception as e:
            logger.error(f"Communication error with client {addr}: {e}")

        finally:
            client_sock.close()
            logger.info(f"Connection with {addr} closed")

if __name__ == '__main__':
    main()
