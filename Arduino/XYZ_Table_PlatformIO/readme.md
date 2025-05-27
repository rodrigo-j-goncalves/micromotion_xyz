# XYZ Camera Positioning System 🎥🛠️

A modular firmware solution for controlling a 3-axis camera positioning table using stepper motors, limit switches, and a command-line interface (CLI). Built using PlatformIO and designed to run on Arduino Mega 2560.

---

## 📦 Project Overview

This system manages a camera or sensor on a mechanical XYZ table using precise stepper motor control. It supports:

- Motion control along X, Y, Z axes
- Limit switch handling with retraction
- CLI over serial for live control and configuration
- LED status indicator
- Structured using an FSM (finite state machine)

---

## 📁 Project Structure

```
XYZ_Table_PlatformIO/
├── src/                    # Source code (.cpp / .h files)
│   ├── CLIService.*        # Command-line interface over serial
│   ├── ControlService.*    # FSM for motion commands (run, move, stop)
│   ├── FancyLED.*          # LED status handler
│   ├── MegaBoard.*         # Hardware abstraction layer
│   ├── Scheduler.*         # Main application scheduler
│   ├── StepperMotors.*     # Axis motor control logic (AccelStepper)
│   └── XyzTable.ino        # Main Arduino entry point
├── lib/                    # (Optional) Custom libraries
├── .pio/                   # PlatformIO build output
├── platformio.ini          # PlatformIO configuration
└── .vscode/                # VS Code build/debug configs
```

---

## ⚙️ Dependencies

- **PlatformIO** (recommended IDE: VS Code)
- **AccelStepper** library

These are automatically handled via `platformio.ini`.

---

## 🛠️ Supported Hardware

- **Microcontroller:** Arduino Mega 2560
- **Motor Drivers:** Compatible with AccelStepper (e.g., A4988, DRV8825)
- **Limit Switches:** Configurable for each axis (X, Y, Z)
- **LED:** Status indicator connected to pin 13

---

## 🔧 CLI Commands

Serial commands can be sent via any terminal (e.g., PlatformIO Serial Monitor, PuTTY):

- `version` → Display firmware version
- `reboot` → Soft reboot the board
- `ram` → Show free RAM in bytes
- `axe X maxSpeed=400 acceleration=200 inverted=true` → Configure axis
- `move all 10` → Move all axes by 10 units
- `run -x` → Run X axis continuously in reverse
- `stop all` → Stop all motion

Use `^` as prefix in serial output for system messages (e.g., `^FSM`, `^SECURITY`).

---

## 🚦 LED Behavior

- **On:** System initializing
- **Pulsing:** System ready
- **Off:** Idle or stopped

---

## ▶️ How to Build & Upload

1. Open the project in VS Code with the PlatformIO extension installed.
2. Connect the Arduino Mega via USB.
3. Use the **Build** and **Upload** buttons in PlatformIO.

Alternatively, from CLI:

```bash
pio run
pio upload
pio device monitor
```

---

## 👨‍💻 Author

**Ignacio Martínez Navajas**  
📧 imnavajas@coit.es

---

## 📄 License

This project is for educational and research purposes. Please credit the original author if reused.