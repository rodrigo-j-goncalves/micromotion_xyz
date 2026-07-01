import socket
import serial
import serial.tools.list_ports
import select
import sys
import logging
from logging.handlers import TimedRotatingFileHandler
from datetime import datetime
from pathlib import Path
from colorama import init, Fore, Style

try:
    import tomllib
except ImportError:
    try:
        import tomli as tomllib
    except ImportError:
        print("Error: Python 3.11+ required, or install tomli: pip install tomli")
        sys.exit(1)

init(autoreset=True)

REPO_ROOT  = Path(__file__).parent.parent.parent
CONFIG_PATH = REPO_ROOT / "config.toml"
LOG_DIR     = Path(__file__).parent / "logs"
VERSION     = (REPO_ROOT / "VERSION").read_text().strip()


def load_config():
    with open(CONFIG_PATH, "rb") as f:
        return tomllib.load(f)


def setup_logger():
    LOG_DIR.mkdir(exist_ok=True)
    logger = logging.getLogger("xyzTableServer")
    logger.setLevel(logging.DEBUG)

    file_handler = TimedRotatingFileHandler(
        LOG_DIR / "xyzTableServer.log",
        when="midnight", backupCount=7, encoding="utf-8"
    )
    file_handler.setFormatter(logging.Formatter("[%(asctime)s] [%(levelname)s] %(message)s"))

    console_handler = logging.StreamHandler()
    console_handler.setFormatter(logging.Formatter("[%(levelname)s] %(message)s"))

    logger.addHandler(file_handler)
    logger.addHandler(console_handler)
    return logger


def print_banner(cfg):
    print(Fore.CYAN + Style.BRIGHT + f"\n=== xyzTableServer v{VERSION} ===")
    print(Fore.YELLOW + "TCP <-> Serial bridge for Arduino Mega")
    print(Fore.BLUE   + f"Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
    print(Fore.GREEN  + f"  Serial port : {Fore.WHITE}{cfg['serial']['port']}")
    print(Fore.GREEN  + f"  Baudrate    : {Fore.WHITE}{cfg['serial']['baudrate']}")
    print(Fore.GREEN  + f"  TCP host    : {Fore.WHITE}{cfg['network']['host']}")
    print(Fore.GREEN  + f"  TCP port    : {Fore.WHITE}{cfg['network']['port']}\n")


def list_serial_ports():
    return [p.device for p in serial.tools.list_ports.comports()]


def stop_motors(ser, logger):
    try:
        ser.write(b"stop\r")
        logger.info("Sent stop command to Arduino")
        print(Fore.YELLOW + "[SAFE] Motors stopped.")
    except Exception as e:
        logger.warning(f"Could not send stop command: {e}")


def serve(cfg, ser, logger):
    host = cfg["network"]["host"]
    port = cfg["network"]["port"]

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((host, port))
    server.listen(1)
    logger.info(f"TCP server listening on {host}:{port}")
    print(Fore.CYAN + f"[INFO] Listening on {host}:{port}")

    while True:
        print(Fore.YELLOW + "\n[WAIT] Waiting for client connection...")
        client_sock, addr = server.accept()
        logger.info(f"Client connected from {addr}")
        print(Fore.CYAN + f"[CONN] Client connected from {addr}")

        try:
            while True:
                # Wait for data from either the TCP client or the Arduino serial port
                readable, _, _ = select.select([client_sock, ser], [], [], 0.01)

                for src in readable:
                    if src is client_sock:
                        data = client_sock.recv(1024)
                        if not data:
                            raise ConnectionResetError("Client closed connection")
                        ser.write(data)
                        logger.debug(f"[TCP->SERIAL] {data!r}")

                    elif src is ser:
                        data = ser.read(ser.in_waiting or 1)
                        if data:
                            client_sock.sendall(data)
                            logger.debug(f"[SERIAL->TCP] {data!r}")

        except (ConnectionResetError, BrokenPipeError):
            logger.info(f"Client {addr} disconnected")
            print(Fore.RED + f"[DISC] Client {addr} disconnected")
        except OSError as e:
            logger.error(f"Socket error with {addr}: {e}")
        finally:
            stop_motors(ser, logger)
            client_sock.close()
            logger.info(f"Connection with {addr} closed")


def main():
    cfg = load_config()
    logger = setup_logger()
    print_banner(cfg)

    serial_port = cfg["serial"]["port"]
    baudrate    = cfg["serial"]["baudrate"]

    try:
        ser = serial.Serial(serial_port, baudrate, timeout=0)
        logger.info(f"Connected to Arduino on {serial_port} at {baudrate} bps")
        print(Fore.GREEN + f"[OK] Arduino connected on {serial_port}")
    except serial.SerialException as e:
        logger.error(f"Could not open serial port '{serial_port}': {e}")
        print(Fore.RED + f"\n[ERROR] Could not open '{serial_port}'")
        print(Fore.YELLOW + "\nAvailable serial ports:")
        ports = list_serial_ports()
        if ports:
            for p in ports:
                print(Fore.CYAN + f"  - {p}")
        else:
            print(Fore.RED + "  No serial ports found. Is the Arduino connected?")
        print(Fore.BLUE + f"\nEdit 'serial.port' in config.toml and restart.\n")
        sys.exit(1)

    try:
        serve(cfg, ser, logger)
    except KeyboardInterrupt:
        print(Fore.CYAN + "\n[EXIT] Server stopped by user.")
        stop_motors(ser, logger)
    finally:
        ser.close()


if __name__ == "__main__":
    main()
