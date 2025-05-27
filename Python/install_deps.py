#!/usr/bin/env python3
"""
xyzTableServer Dependency Installer

Checks for and installs all required Python packages
used by the client and server components of the project.
"""

import subprocess
import sys
import importlib
from colorama import init, Fore

# Auto-reset color output after each print
init(autoreset=True)

# Required modules with their pip install names
REQUIRED_PACKAGES = {
    "serial": "pyserial",
    "colorama": "colorama",
    "dotenv": "python-dotenv",
}

def install_package(package_name):
    print(Fore.YELLOW + f"Installing {package_name}...")
    result = subprocess.run([sys.executable, "-m", "pip", "install", package_name])
    return result.returncode == 0

def check_and_install():
    print(Fore.CYAN + "\n=== xyzTableServer Dependency Installer ===\n")
    all_installed = True

    for module_name, pip_name in REQUIRED_PACKAGES.items():
        try:
            importlib.import_module(module_name)
            print(Fore.GREEN + f"[OK] {pip_name} is already installed.")
        except ImportError:
            print(Fore.RED + f"[MISSING] {pip_name} is not installed.")
            success = install_package(pip_name)
            if success:
                print(Fore.GREEN + f"[INSTALLED] {pip_name} installed successfully.")
            else:
                print(Fore.RED + f"[FAILED] Could not install {pip_name}.")
                all_installed = False

    if all_installed:
        print(Fore.GREEN + "\nAll dependencies are ready! ✅\n")
    else:
        print(Fore.RED + "\nSome packages could not be installed. Please check above errors. ❌\n")

if __name__ == "__main__":
    check_and_install()
