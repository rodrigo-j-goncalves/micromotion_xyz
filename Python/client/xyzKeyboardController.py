import asyncio
import sys
import re
import logging
from logging.handlers import TimedRotatingFileHandler
from time import time
from pathlib import Path
from itertools import cycle
from colorama import init, Fore, Style
from pynput import keyboard

try:
    import tomllib
except ImportError:
    try:
        import tomli as tomllib
    except ImportError:
        print("Error: Python 3.11+ required, or install tomli: pip install tomli")
        sys.exit(1)

init(autoreset=True)

REPO_ROOT   = Path(__file__).parent.parent.parent
CONFIG_PATH = REPO_ROOT / "config.toml"
VERSION     = (REPO_ROOT / "VERSION").read_text().strip()
LOG_DIR     = Path(__file__).parent / "logs"


def setup_logger():
    LOG_DIR.mkdir(exist_ok=True)
    logger = logging.getLogger("xyzClient")
    logger.setLevel(logging.DEBUG)
    handler = TimedRotatingFileHandler(
        LOG_DIR / "xyzKeyboardController.log",
        when="midnight", backupCount=30, encoding="utf-8"
    )
    handler.setFormatter(logging.Formatter("[%(asctime)s] %(message)s"))
    logger.addHandler(handler)
    return logger


def load_config():
    with open(CONFIG_PATH, "rb") as f:
        return tomllib.load(f)


def parse_key(key_str):
    """Convert a key name from config.toml to a pynput key object."""
    if len(key_str) == 1:
        return keyboard.KeyCode.from_char(key_str)
    try:
        return getattr(keyboard.Key, key_str.lower())
    except AttributeError:
        raise ValueError(f"Invalid key name in config.toml: '{key_str}'")


def load_keymap(cfg):
    keys = cfg["keys"]
    axes = cfg["axes"]
    return {
        parse_key(keys["up"]):     (axes["up_axis"],      axes["up_sign"]),
        parse_key(keys["down"]):   (axes["down_axis"],    axes["down_sign"]),
        parse_key(keys["left"]):   (axes["left_axis"],    axes["left_sign"]),
        parse_key(keys["right"]):  (axes["right_axis"],   axes["right_sign"]),
        parse_key(keys["z_up"]):   (axes["z_up_axis"],    axes["z_up_sign"]),
        parse_key(keys["z_down"]): (axes["z_down_axis"],  axes["z_down_sign"]),
    }


def load_speeds(cfg):
    speeds = {}
    for axis, values in cfg["speeds"].items():
        if values:
            speeds[axis] = {
                "list":    values,
                "cycle":   cycle(values),
                "current": values[0],
            }
    return speeds


def strip_ansi(text):
    return re.sub(r'\x1b[^m]*m', '', text)


def print_menu(cfg):
    keys  = cfg["keys"]
    width = 52
    title = f"MICROMOTION LAB  v{VERSION}"

    entries = [
        (Fore.GREEN   + f"↑/↓",          f"eje X  (hacia/desde cámara)  [{keys['up']}/{keys['down']}]"),
        (Fore.GREEN   + f"←/→",          f"eje Y  (izq/der cámara)       [{keys['left']}/{keys['right']}]"),
        (Fore.MAGENTA + f"a/z",           f"eje Z  (arriba/abajo)          [{keys['z_up']}/{keys['z_down']}]"),
        (Fore.YELLOW  + "Shift + X/Y/Z", "ciclar velocidad del eje"),
        (Fore.RED     + "ESC",           "parar todos los motores"),
        (Fore.RED     + "Q",             "salir del programa"),
    ]

    print(Fore.CYAN + "╭" + "─" * (width - 2) + "╮")
    print(Fore.CYAN + "│" + Fore.YELLOW + Style.BRIGHT + title.center(width - 2) + Style.RESET_ALL + Fore.CYAN + "│")
    print(Fore.CYAN + "├" + "─" * (width - 2) + "┤")
    for key_label, desc in entries:
        text = f" {key_label}{Fore.WHITE} → {desc}"
        padding = width - 2 - len(strip_ansi(text))
        print(Fore.CYAN + "│" + text + " " * max(0, padding - 1) + Fore.CYAN + "│")
    print(Fore.CYAN + "╰" + "─" * (width - 2) + "╯\n")


class XYZClient:
    def __init__(self, host, port, log=None):
        self.host   = host
        self.port   = port
        self.reader = None
        self.writer = None
        self._listen_task = None
        self._log   = log

    async def connect(self):
        if self.writer is not None:
            return
        self.reader, self.writer = await asyncio.open_connection(self.host, self.port)
        print(Fore.CYAN + f"[Connected] {self.host}:{self.port}")
        self._listen_task = asyncio.create_task(self._listen(self._log))

    async def send(self, cmd):
        await self.connect()
        self.writer.write(f"{cmd}\r".encode())
        await self.writer.drain()

    async def _listen(self, log=None):
        """Print messages arriving from the server (Arduino responses and events)."""
        try:
            while True:
                line = await self.reader.readline()
                if not line:
                    print(Fore.RED + "[Server closed connection]")
                    if log:
                        log.warning("Server closed connection")
                    break
                msg = line.decode(errors="ignore").strip()
                if not msg:
                    continue
                if msg.startswith("^"):
                    tag = msg[1:]
                    # Highlight and log safety-critical messages
                    if any(w in tag for w in ("LIMIT", "RETRACT", "SECURITY")):
                        print(Fore.RED + Style.BRIGHT + f"[!] {tag}")
                        if log:
                            log.warning(f"FIRMWARE EVENT: {tag}")
                    else:
                        print(Fore.MAGENTA + f"[>] {tag}")
                        if log:
                            log.info(f"FIRMWARE: {tag}")
        except asyncio.CancelledError:
            pass
        except Exception as e:
            print(Fore.RED + f"[Receive error] {e}")
            if log:
                log.error(f"Receive error: {e}")

    async def aclose(self):
        if self._listen_task:
            self._listen_task.cancel()
        if self.writer:
            self.writer.close()
            try:
                await self.writer.wait_closed()
            except Exception:
                pass


async def main():
    cfg    = load_config()
    log    = setup_logger()
    keymap = load_keymap(cfg)
    speeds = load_speeds(cfg)
    client = XYZClient(cfg["network"]["host"], cfg["network"]["port"], log=log)
    loop   = asyncio.get_running_loop()

    log.info(f"=== Session started  v{VERSION} ===")
    log.info(f"Connecting to {cfg['network']['host']}:{cfg['network']['port']}")

    active_keys         = {}                              # key → axis currently running
    press_times         = {}                              # key → timestamp of press
    modifiers           = set()                           # active modifiers (shift)
    shift_speed_fired   = {"x": False, "y": False, "z": False}  # debounce speed toggle

    async def _send(cmd):
        try:
            await client.send(cmd)
        except Exception as e:
            print(Fore.RED + f"[Send error] {e}")
            log.error(f"Send error: {e}")

    def run_axis(key):
        if key in active_keys:
            return
        axis, sign = keymap[key]
        active_keys[key] = axis
        press_times[key]  = time()
        cmd = f"run {axis}" if sign == "+" else f"run -{axis}"
        asyncio.run_coroutine_threadsafe(_send(cmd), loop)
        print(Fore.YELLOW + f"[RUN] {axis.upper()}  {'(+)' if sign == '+' else '(-)'}")
        log.info(f"RUN {axis.upper()} {'(+)' if sign == '+' else '(-)'}")

    def stop_axis(key):
        axis = active_keys.pop(key, None)
        if axis is None:
            return
        duration = time() - press_times.pop(key, time())
        # Always send stop — regardless of how long the key was held
        asyncio.run_coroutine_threadsafe(_send(f"stop {axis}"), loop)
        flag = "  [long press]" if duration > 3.0 else ""
        print(Fore.GREEN + f"[STOP] {axis.upper()}  {duration:.2f}s{flag}")
        log.info(f"STOP {axis.upper()}  duration={duration:.2f}s{flag.strip()}")

    def stop_all():
        for k in list(active_keys):
            active_keys.pop(k, None)
            press_times.pop(k, None)
        asyncio.run_coroutine_threadsafe(_send("stop"), loop)
        log.info("STOP ALL")

    def toggle_speed(axis):
        if axis not in speeds:
            print(Fore.RED + f"[Speed] No speeds defined for axis {axis.upper()} in config.toml")
            return
        speed = next(speeds[axis]["cycle"])
        speeds[axis]["current"] = speed
        asyncio.run_coroutine_threadsafe(_send(f"axe {axis} maxSpeed={speed}"), loop)
        idx   = speeds[axis]["list"].index(speed) + 1
        total = len(speeds[axis]["list"])
        print(Fore.CYAN + f"[Speed] {axis.upper()} → {speed} steps/s  ({idx}/{total})")
        log.info(f"SPEED {axis.upper()} -> {speed} steps/s ({idx}/{total})")

    def on_press(key):
        # --- Shift modifier ---
        if key in (keyboard.Key.shift, keyboard.Key.shift_r):
            modifiers.add("shift")
            return

        # --- Q: clean quit ---
        if isinstance(key, keyboard.KeyCode) and key.char == "q":
            print(Fore.CYAN + "\n[EXIT] Stopping motors and quitting...")
            log.info("EXIT requested by user (Q)")
            stop_all()
            return False  # stops the pynput listener

        # --- Shift + X/Y/Z: cycle speed (fire once per press, not on key repeat) ---
        if "shift" in modifiers and isinstance(key, keyboard.KeyCode) and key.char:
            char = key.char.lower()
            if char in ("x", "y", "z"):
                if not shift_speed_fired[char]:
                    shift_speed_fired[char] = True
                    toggle_speed(char)
                return  # do not move while changing speed

        # --- Movement keys ---
        if key in keymap:
            run_axis(key)

    def on_release(key):
        # --- Shift released: clear modifier and speed-toggle debounce ---
        if key in (keyboard.Key.shift, keyboard.Key.shift_r):
            modifiers.discard("shift")
            for axis in shift_speed_fired:
                shift_speed_fired[axis] = False
            return

        # --- ESC: emergency stop all axes ---
        if key == keyboard.Key.esc:
            print(Fore.RED + Style.BRIGHT + "[ESC] Emergency stop!")
            log.warning("ESC emergency stop triggered by user")
            stop_all()
            return

        # --- Release movement key: stop that axis ---
        if key in active_keys:
            stop_axis(key)

    print_menu(cfg)

    with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
        try:
            await loop.run_in_executor(None, listener.join)
        finally:
            stop_all()
            await asyncio.sleep(0.1)   # let the stop command drain
            await client.aclose()
            log.info("=== Session ended ===")
            print(Fore.CYAN + "[EXIT] Done.")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print(Fore.RED + "\n[EXIT] Interrupted.")
