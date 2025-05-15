
#!/usr/bin/env python3
"""
xyzTableClient.py

Interactive CLI with argparse-style command dispatch.
Keeps the TCP connection open and supports structured commands.
"""

import socket
import os
import sys
import argparse
from dotenv import load_dotenv
from colorama import init, Fore, Style

init(autoreset=True)
load_dotenv()

DEFAULT_HOST = os.getenv("HOST", "127.0.0.1")
DEFAULT_PORT = int(os.getenv("PORT", "5000"))
VALID_AXES = {"X", "Y", "Z", "ALL"}

def send(sock, message):
    try:
        sock.sendall((message + "\r").encode())  # usa \\r como el servidor espera
        raw = sock.recv(4096)
        response = raw.decode("utf-8", errors="ignore").strip(" \r\n>")
        if response:
            print(response)
    except Exception as e:
        print(Fore.RED + f"Send failed: {e}")

def print_help(command=None):
    parser = argparse.ArgumentParser(prog="xyz", description="XYZ Table CLI", add_help=True)
    subparsers = parser.add_subparsers(dest="command")

    move = subparsers.add_parser("move", help="Move axis")
    move.add_argument("axis", choices=VALID_AXES - {"ALL"})
    move.add_argument("steps", type=int)

    run = subparsers.add_parser("run", help="Start continuous motion")
    run.add_argument("axis", nargs="?", choices=VALID_AXES)

    axe = subparsers.add_parser("axe", help="Set axis parameter")
    axe.add_argument("axis", choices=VALID_AXES - {"ALL"})
    axe.add_argument("param")
    axe.add_argument("value")

    for cmd in ["stop", "version", "reboot", "ram", "exit", "help", "interactive"]:
        subparsers.add_parser(cmd)

    raw = subparsers.add_parser("raw", help="Send raw command")
    raw.add_argument("text", nargs=argparse.REMAINDER)

    if command:
        try:
            parser.parse_args([command, "--help"])
        except SystemExit:
            pass
    else:
        parser.print_help()


def handle_command(sock, line):
    # Special case: "help <command>" -> "<command> --help"
    if line.startswith("help "):
        parts = line.split(maxsplit=1)
        line = f"{parts[1]} --help"

    parser = argparse.ArgumentParser(prog="xyz", description="XYZ Table CLI", add_help=False)
    subparsers = parser.add_subparsers(dest="command")

    # Subcommands
    move = subparsers.add_parser("move")
    move.add_argument("axis", choices=VALID_AXES - {"ALL"})
    move.add_argument("steps", type=int)

    run = subparsers.add_parser("run")
    run.add_argument("axis", nargs="?", choices=VALID_AXES)

    axe = subparsers.add_parser("axe")
    axe.add_argument("axis", choices=VALID_AXES - {"ALL"})
    axe.add_argument("param")
    axe.add_argument("value")

    for cmd in ["stop", "version", "reboot", "ram", "exit", "help", "interactive"]:
        subparsers.add_parser(cmd)

    raw = subparsers.add_parser("raw")
    raw.add_argument("text", nargs=argparse.REMAINDER)

    try:
        args = parser.parse_args(line.split())
    except SystemExit:
        return

    # Command dispatch
    if args.command == "move":
        send(sock, f"move {args.axis} {args.steps}")
    elif args.command == "run":
        if args.axis:
            send(sock, f"run {args.axis}")
        else:
            send(sock, "run")
    elif args.command == "axe":
        send(sock, f"axe {args.axis} {args.param} {args.value}")
    elif args.command == "stop":
        send(sock, "stop")
    elif args.command == "version":
        send(sock, "version")
    elif args.command == "reboot":
        send(sock, "reboot")
    elif args.command == "ram":
        send(sock, "ram")
    elif args.command == "raw":
        send(sock, " ".join(args.text))
    elif args.command == "help":
        print_help()
    elif args.command == "interactive":
        interactive_motion(sock)
    elif args.command == "exit":
        print(Fore.CYAN + "Closing connection.")
        sys.exit(0)


def interactive_motion(sock):
    print(Fore.CYAN + "\nEntering interactive motion mode. Use arrow keys to move axes.")
    print(Fore.CYAN + "  ↑  → Z+	↓  → Z-")
    print(Fore.CYAN + "  ←  → X-	→  → X+")
    print(Fore.CYAN + "  SHIFT + arrows → Y axis")
    print(Fore.YELLOW + "Press 'q' to quit interactive mode.\n")

    try:
        import keyboard  # pip install keyboard
    except ImportError:
        print(Fore.RED + "keyboard module not installed. Please install it with `pip install keyboard`.")
        return

    # Map keys to axis and direction
    key_map = {
        "up": ("Z", "run"),
        "down": ("Z", "run"),
        "left": ("X", "run"),
        "right": ("X", "run"),
        "shift+up": ("Y", "run"),
        "shift+down": ("Y", "run"),
        "shift+left": ("Y", "run"),
        "shift+right": ("Y", "run"),
    }

    # Determine direction (sign not used here, just example mapping)
    active_keys = {}

    def on_press(e):
        key = e.name
        if e.event_type == "down":
            if key == "q":
                print(Fore.CYAN + "\nExiting interactive motion mode.")
                keyboard.unhook_all()
                return False
            for k, (axis, cmd) in key_map.items():
                if keyboard.is_pressed(k) and k not in active_keys:
                    send(sock, f"run {axis}")
                    active_keys[k] = axis

    def on_release(e):
        key = e.name
        to_remove = []
        for k in active_keys:
            if not keyboard.is_pressed(k):
                axis = active_keys[k]
                send(sock, f"stop {axis}")
                to_remove.append(k)
        for k in to_remove:
            del active_keys[k]

    print(Fore.GREEN + "Ready. Use keys...")

    keyboard.hook(on_press)
    keyboard.hook(on_release)

    # Keep the loop alive
    try:
        while True:
            keyboard.wait("q")
            break
    except KeyboardInterrupt:
        pass
    finally:
        for k, axis in active_keys.items():
            send(sock, f"stop {axis}")
        print(Fore.CYAN + "Stopped all motion.")


def main():
    try:
        with socket.create_connection((DEFAULT_HOST, DEFAULT_PORT), timeout=5) as sock:
            print(Fore.CYAN + Style.BRIGHT + f"Connected to {DEFAULT_HOST}:{DEFAULT_PORT}")
            while True:
                try:
                    line = input(Fore.YELLOW + "xyz> ").strip()
                    if not line:
                        continue
                    handle_command(sock, line)
                except (KeyboardInterrupt, EOFError):
                    print(Fore.CYAN + "\nExiting.")
                    break
    except Exception as e:
        print(Fore.RED + f"Could not connect to server: {e}")

if __name__ == "__main__":
    main()
