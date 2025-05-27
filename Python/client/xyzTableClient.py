#!/usr/bin/env python3
"""
Cross-platform CLI for XYZ-Table controller (TCP→Serial bridge).

• asyncio command/response client (+ '^' async notifications)
• Interactive motion mode (arrows, Shift+arrows, ESC = stop, q = quit)
  – Windows  : usa msvcrt (no requiere privilegios)
  – Linux/mac: usa termios/tty raw mode
Requisitos:
    pip install colorama python-dotenv
"""

from __future__ import annotations
import asyncio, os, sys, json, time, platform, signal, threading, select
from typing import Awaitable, Callable, Dict, List, Optional
from colorama import Fore, init
from dotenv import load_dotenv

# ─────────── Config ────────────────────────────────────────────────
load_dotenv();
init(autoreset=True)
HOST = os.getenv("HOST", "127.0.0.1")
PORT = int(os.getenv("PORT", "5000"))
ETX = b"\x03"
VALID_AXES = {"x", "y", "z", "all"}


# ─────────── TCP client ────────────────────────────────────────────
class XYZClient:
    """TCP client with command/response and '^' async events."""

    def __init__(self, host: str = HOST, port: int = PORT) -> None:
        self._host, self._port = host, port
        self._reader: Optional[asyncio.StreamReader] = None
        self._writer: Optional[asyncio.StreamWriter] = None
        self._pending: Optional[asyncio.Future[str]] = None
        self._listeners: List[Callable[[str], Awaitable[None]]] = []

    async def connect(self) -> None:
        if self._reader: return
        self._reader, self._writer = await asyncio.open_connection(self._host, self._port)
        print(Fore.CYAN + f"[xyz] connected to {self._host}:{self._port}")
        asyncio.create_task(self._read_loop())

    async def _read_loop(self) -> None:
        buf = b""
        while True:
            chunk = await self._reader.read(1024)
            if not chunk:
                print(Fore.RED + "[xyz] peer closed");
                break
            buf += chunk
            while ETX in buf:
                frame, buf = buf.split(ETX, 1)
                line = frame.decode(errors="ignore").strip()
                if line.startswith("^"):
                    for cb in self._listeners: asyncio.create_task(cb(line[1:]))
                elif self._pending and not self._pending.done():
                    self._pending.set_result(line)
                else:
                    print(Fore.YELLOW + f"[xyz] unsolicited: {line}")

    async def send(self, cmd: str, *, timeout: float = 5.0) -> str:
        await self.connect()
        if self._pending and not self._pending.done():
            raise RuntimeError("command pending")
        loop = asyncio.get_running_loop()
        self._pending = loop.create_future()
        self._writer.write(f"{cmd}\r".encode());
        await self._writer.drain()
        return await asyncio.wait_for(self._pending, timeout)

    def add_async_listener(self, cb):
        self._listeners.append(cb)

    async def aclose(self):
        if self._writer: self._writer.close(); await self._writer.wait_closed()


# ─────────── Interactive motion (raw keyboard) ─────────────────────
def interactive_motion(client: XYZClient, loop: asyncio.AbstractEventLoop) -> None:
    import string
    active: Dict[str, str] = {}
    stop_evt = threading.Event()
    keymap = load_keymap()
    axis_speed_state = {"x": "default", "y": "default", "z": "default"}

    async def _send(cmd: str):
        try:
            await client.send(cmd, timeout=0.3)
        except Exception:
            pass

    def axis_for(key: str, ctrl: bool) -> Optional[tuple[str, str]]:
        k = f"CTRL_{key}" if ctrl and key in ("UP", "DOWN") else key
        return keymap.get(k)

    def start(axis: str, sign: str, tok: str):
        if tok in active:
            return
        active[tok] = axis
        asyncio.run_coroutine_threadsafe(_send(f"run {sign}{axis}"), loop)
        loop.call_soon_threadsafe(print, f"{axis.upper()} {'+' if sign == '' else '-'} start")

    def stop(tok: str):
        axis = active.pop(tok, None)
        if axis:
            asyncio.run_coroutine_threadsafe(_send(f"stop {axis}"), loop)
            loop.call_soon_threadsafe(print, f"{axis.upper()} stop")

    def toggle_speed(axis: str):
        state = axis_speed_state[axis]
        if state == "default":
            speed = max_speed_for(axis)
            axis_speed_state[axis] = "max"
        else:
            speed = default_speed_for(axis)
            axis_speed_state[axis] = "default"
        if speed:
            asyncio.run_coroutine_threadsafe(_send(f"axe {axis} maxSpeed={speed}"), loop)
            loop.call_soon_threadsafe(print, Fore.CYAN + f"[Speed] {axis.upper()} axis → maxSpeed={speed}")

    loop.call_soon_threadsafe(
        print, Fore.CYAN +
        "\n[interactive mode]  (customizable via .env)\n"
        "  ←/→, ↑/↓, Ctrl+↑/↓ mapped to axes\n"
        "  Shift + X/Y/Z = toggle speed\n"
        "  ESC = Stop all  |  q = Quit interactive mode\n"
    )

    if os.name == "nt":  # Windows
        import ctypes, msvcrt
        user32 = ctypes.windll.user32
        VK = {
            "UP": 0x26, "DOWN": 0x28, "LEFT": 0x25, "RIGHT": 0x27,
            "ESC": 0x1B, "Q": 0x51, "CTRL": 0x11, "SHIFT": 0x10
        }

        pressed = {k: False for k in ("UP", "DOWN", "LEFT", "RIGHT")}
        shift_toggle_pressed = {"x": False, "y": False, "z": False}
        esc_pressed = False

        def vk_down(code: int) -> bool:
            return bool(user32.GetAsyncKeyState(code) & 0x8000)

        while not stop_evt.is_set():
            shift = vk_down(VK["SHIFT"])
            for axis_char in ("x", "y", "z"):
                vk_char = ord(axis_char.upper())
                is_down = vk_down(vk_char) and shift
                if is_down and not shift_toggle_pressed[axis_char]:
                    shift_toggle_pressed[axis_char] = True
                    toggle_speed(axis_char)
                elif not is_down:
                    shift_toggle_pressed[axis_char] = False

            if vk_down(VK["Q"]):
                stop_evt.set()
                break

            is_esc_down = vk_down(VK["ESC"])
            if is_esc_down and not esc_pressed:
                esc_pressed = True
                print(Fore.CYAN + "[ESC] Stop all motors")
                for t in list(active): stop(t)
                asyncio.run_coroutine_threadsafe(_send("stop"), loop)
            elif not is_esc_down:
                esc_pressed = False

            ctrl = vk_down(VK["CTRL"])
            for key in ("UP", "DOWN", "LEFT", "RIGHT"):
                is_down = vk_down(VK[key])
                token = f"C+{key}" if ctrl and key in ("UP", "DOWN") else key
                if is_down and not pressed[key]:
                    ax_sign = axis_for(key, ctrl)
                    if ax_sign:
                        axis, sign = ax_sign
                        start(axis, sign, token)
                elif not is_down and pressed[key]:
                    stop(token)
                pressed[key] = is_down
            time.sleep(0.03)

    else:  # Unix
        import termios, tty
        fd = sys.stdin.fileno()
        orig_attr = termios.tcgetattr(fd)
        tty.setraw(fd)
        shift_toggle_pressed = {"x": False, "y": False, "z": False}
        try:
            while not stop_evt.is_set():
                r, _, _ = select.select([fd], [], [], 0.05)
                if not r:
                    continue
                seq = os.read(fd, 1)
                if seq == b"q":
                    stop_evt.set()
                    break
                if seq in b"XYZ":
                    axis = seq.decode().lower()
                    if not shift_toggle_pressed[axis]:
                        shift_toggle_pressed[axis] = True
                        toggle_speed(axis)
                    continue
                for axis in shift_toggle_pressed:
                    shift_toggle_pressed[axis] = False
                if seq == b"\x1b":
                    next1 = os.read(fd, 1)
                    if next1 == b"[":
                        next2 = os.read(fd, 1)
                        if next2 in b"ABCD":
                            key = {"A": "UP", "B": "DOWN", "C": "RIGHT", "D": "LEFT"}[next2.decode()]
                            token = key
                            ax_sign = axis_for(key, False)
                            if ax_sign:
                                axis, sign = ax_sign
                                start(axis, sign, token)
                                stop(token)
                        elif next2 == b"1":
                            _ = os.read(fd, 1)  # expect ;
                            mod = os.read(fd, 1)
                            arrow = os.read(fd, 1)
                            if mod == b"5" and arrow in b"AB":
                                key = {"A": "UP", "B": "DOWN"}[arrow.decode()]
                                token = f"C+{key}"
                                ax_sign = axis_for(key, True)
                                if ax_sign:
                                    axis, sign = ax_sign
                                    start(axis, sign, token)
                                    stop(token)
                    else:
                        print(Fore.CYAN + "[ESC] Stop all motors")
                        for t in list(active): stop(t)
                        asyncio.run_coroutine_threadsafe(_send("stop"), loop)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, orig_attr)

    for t in list(active):
        stop(t)
    asyncio.run_coroutine_threadsafe(_send("stop"), loop)
    loop.call_soon_threadsafe(print, Fore.CYAN + "Stopped all motion.")

# ─────────── Helpers / CLI loop ─────────────────────────────────────
def help_text():
    print(Fore.CYAN +
          "\nCommands:"
          "\n  move <axis> <steps>"
          "\n  run [axis] | stop [axis]"
          "\n  axe <axis>           (get params)"
          "\n  axe <axis> p=v       (set param)"
          "\n  version | reboot | ram"
          "\n  interactive"
          "\n  help | exit | quit\n")

async def cli(client: XYZClient):
    await client.connect()
    client.add_async_listener(lambda m: print(Fore.MAGENTA + f"[Async] {m}"))
    help_text()
    loop = asyncio.get_running_loop()
    while True:
        try:
            line = await loop.run_in_executor(None, lambda: input(Fore.YELLOW + "xyz> ").strip())
        except (EOFError, KeyboardInterrupt):
            break
        if not line: continue
        cmd, *args = line.split();
        cmd = cmd.lower()
        if cmd in {"exit", "quit"}: break
        if cmd == "help": help_text(); continue
        if cmd == "interactive":
            await loop.run_in_executor(None, interactive_motion, client, loop)
            continue
        try:
            if cmd == "move" and len(args) == 2:
                axis, steps = args
                if axis not in VALID_AXES - {"all"}:
                    print(Fore.RED + "axis must be x,y,z");
                    continue
                reply = await client.send(f"move {axis} {steps}")
            elif cmd in {"run", "stop"}:
                axis = args[0] if args else ""
                reply = await client.send(f"{cmd} {axis}")
            elif cmd in {"reboot", "version", "ram"}:
                reply = await client.send(cmd)
            elif cmd == "axe":
                if len(args) == 1:
                    reply = await client.send(f"axe {args[0]}")
                elif len(args) == 2 and "=" in args[1]:
                    p, v = args[1].split("=", 1)
                    reply = await client.send(f"axe {args[0]} {p}={v}")
                else:
                    print(Fore.RED + "axe usage");
                    continue
            else:
                print(Fore.RED + "unknown");
                continue
            try:
                print(Fore.GREEN + json.dumps(json.loads(reply), indent=2))
            except:
                print(Fore.GREEN + reply)
        except asyncio.TimeoutError:
            print(Fore.RED + "timeout")
        except Exception as e:
            print(Fore.RED + str(e))

def load_keymap() -> dict:
    keymap = {}
    for key in ["LEFT", "RIGHT", "UP", "DOWN", "CTRL_UP", "CTRL_DOWN"]:
        val = os.getenv(f"KEYMAP_{key}")
        if val:
            axis, sign = val.split(",")
            keymap[key] = (axis.strip(), "" if sign.strip() == "+" else "-")
    return keymap

def max_speed_for(axis: str) -> Optional[str]:
    return os.getenv(f"MAXSPEED_{axis.upper()}")

def default_speed_for(axis: str) -> Optional[str]:
    return os.getenv(f"DEFAULTSPEED_{axis.upper()}")

# ─────────── main ───────────────────────────────────────────────────
async def _main():
    client = XYZClient()
    if platform.system() != "Windows":
        loop = asyncio.get_running_loop()
        for sig in (signal.SIGINT, signal.SIGTERM):
            try:
                loop.add_signal_handler(sig, lambda: asyncio.create_task(client.aclose()))
            except NotImplementedError:
                pass
    try:
        await cli(client)
    finally:
        await client.aclose()


if __name__ == "__main__":
    try:
        asyncio.run(_main())
    except KeyboardInterrupt:
        pass
