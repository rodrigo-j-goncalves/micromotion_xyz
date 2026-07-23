# micromotion_xyz — v1.0.1

A multi-platform, 3-axis motorized table controller for microscopy/videography.

- Gives fine control over the position (in a 3D space) of a capture device (eg. camera) or a sample (when the camera is fixed).
- Designed for remote operation: the user does not need to be tethered to the hardware. 
- Manual mode: the operator moves the motors manually, in real time. This is useful for capturing events in live samples, but it's time-consuming.
- Automatic/programmable: an arbitrary sequence of movements and positions is pre-defined. Good for reproducibility and long-duration experiments.

---

## System Architecture

Three machines work together:

```
┌─────────────────────────────────────────────────────────────────────┐
│  PC (any computer on the university network)                        │
│  xyzKeyboardController.py  ←  keyboard jogging / speed control      │
│  browser  ←  camera live feed                                       │
└───────────────────────────┬─────────────────────────────────────────┘
                            │  TCP  (port 5000)
┌───────────────────────────┴─────────────────────────────────────────┐
│  Raspberry Pi (in the lab, always on)                               │
│  xyzTableServer.py  ←  TCP ↔ Serial bridge                          │
└───────────────────────────┬─────────────────────────────────────────┘
                            │  USB serial  (115200 baud)
┌───────────────────────────┴─────────────────────────────────────────┐
│  Arduino Mega 2560 (attached to the Raspi via USB)                  │
│  XYZ_Table_PlatformIO firmware  ←  AccelStepper + limit switches    │
│  → 3 × stepper driver (A4988/DRV8825) → motors X, Y, Z              │
└─────────────────────────────────────────────────────────────────────┘
```

**Axis convention** (from the camera's point of view):

| Axis | Direction        | Default keys |
|------|-----------------|--------------|
| X    | depth / focus   | ↑ / ↓        |
| Y    | left / right    | ← / →        |
| Z    | up / down       | a / z        |

Limit switches are safety-critical — the operator is never watching the hardware.
When a switch triggers, that axis stops immediately, retracts a few steps, and the other axes are unaffected.

---

## Repository Layout

```
micromotion_xyz_v1.0.1/
├── config.toml                      ← single config for the whole system
├── VERSION                          ← "1.0.1"
├── README.md                        ← this file
│
├── Arduino/
│   └── XYZ_Table_PlatformIO/
│       ├── platformio.ini
│       └── src/
│           ├── XyzTable.ino         ← main sketch (setup/loop)
│           ├── MegaBoard.*          ← serial helpers (Print/Println/ETX)
│           ├── StepperMotors.*      ← motor + ISR + limit switch logic
│           ├── ControlService.*     ← FSM: IDLE / MOVING_STEPS / MOVING_CONTINUOUS
│           ├── CLIService.*         ← registers CLI commands
│           ├── Cmd.*               ← serial command parser
│           ├── Scheduler.*          ← simple task scheduler
│           └── FancyLED.*           ← status LED
│
└── Python/
    ├── requirements.txt
    ├── setup_venv.sh                ← creates venv at repo root
    ├── server/
    │   └── xyzTableServer.py        ← runs on the Raspberry Pi
    └── client/
        ├── xyzKeyboardController.py ← runs on the operator's PC
        └── logs/                    ← auto-created; one log file per day
```

---

## Configuration

**`config.toml`** at the repo root is the single file you need to edit.
Restart the server and the client after any change; Arduino motor settings
require re-flashing the firmware.

Key sections:

| Section     | What it controls                                   |
|-------------|----------------------------------------------------|
| `[network]` | Raspberry Pi IP and TCP port                       |
| `[serial]`  | USB serial port on the Raspi (`/dev/ttyACM0` etc.) |
| `[keys]`    | Key bindings for all six motion directions         |
| `[axes]`    | Which motor axis each key controls and its sign    |
| `[speeds]`  | Speed levels per axis (Shift+X/Y/Z cycles through) |
| `[motors.*]`| Max speed, acceleration, steps/unit, direction     |
| `[safety]`  | Retract distance, limit debounce, idle timeout     |

---

## Keyboard Controls (default)

| Key          | Action                                  |
|--------------|-----------------------------------------|
| ↑            | X+ (focus in / camera approaches)      |
| ↓            | X− (focus out)                          |
| ←            | Y+ (camera left)                        |
| →            | Y− (camera right)                       |
| a            | Z− (camera up)                          |
| z            | Z+ (camera down)                        |
| Hold key     | Motor runs continuously until released  |
| Shift+X      | Cycle X speed (slow → fast → slow …)   |
| Shift+Y      | Cycle Y speed                           |
| Shift+Z      | Cycle Z speed                           |
| ESC          | Emergency stop — all axes               |
| Q            | Quit client (also stops all axes)       |

Diagonal movement works: hold two keys simultaneously — each axis runs
independently via AccelStepper's `runAll()`.

---

## Serial Command Reference

The server forwards these commands to the Arduino over USB serial.
You can also type them directly in a serial terminal (115200 baud).

| Command                     | Description                                      |
|-----------------------------|--------------------------------------------------|
| `run x`                     | Run X axis forward continuously                  |
| `run -x`                    | Run X axis in reverse continuously               |
| `run y` / `run -y`          | Same for Y                                       |
| `run z` / `run -z`          | Same for Z                                       |
| `run all` / `run -all`      | Run all axes simultaneously                      |
| `stop x`                    | Stop X axis                                      |
| `stop all`                  | Stop all axes and disable motors                 |
| `move x 50`                 | Move X by 50 units (relative)                    |
| `move x 10 y -5 z 2`        | Move multiple axes (decimals allowed: `x 1.5`)   |
| `move all 100`              | Move all axes by the same distance               |
| `axe X`                     | Print X axis settings as JSON                    |
| `axe X maxSpeed=500`        | Change X max speed at runtime                    |
| `version`                   | Print firmware name and version                  |
| `ram`                       | Print free RAM (bytes)                           |
| `reboot`                    | Software reboot the Arduino                      |

All responses end with ETX (0x03) so the server knows when a reply is complete.
Limit-switch events and safety messages are prefixed with `^` and streamed
to the client as they occur.

---

## Deployment

### Step 1 — Flash the Arduino (once, or after firmware changes)

Done on **any PC with PlatformIO** (can be the Raspi or your PC):

```bash
cd Arduino/XYZ_Table_PlatformIO
pio run --target upload
```

Verify in a serial monitor (115200 baud): type `version`, expect `XYZ-Table_v1.0.1`.

### Step 2 — Set up the Raspberry Pi (once per machine)

Copy the entire `micromotion_xyz_v1.0.1/` folder to the Raspi (e.g. `~/micromotion_xyz_v1.0.1/`).

```bash
cd ~/micromotion_xyz_v1.0.1
bash Python/setup_venv.sh       # creates venv/ at repo root
```

Edit `config.toml`:
- `[network] host` — set to the Raspi's IP address (find it with `hostname -I`)
- `[serial] port` — check with `ls /dev/ttyACM* /dev/ttyUSB*` while Arduino is connected

Start the server (keep it running in the background or in a tmux session):

```bash
source venv/bin/activate
python3 Python/server/xyzTableServer.py
```

You should see: `XYZ Table Server v1.0.1 — listening on <raspi IP>:5000`.

### Step 3 — Run the client on the operator's PC

Copy the entire `micromotion_xyz_v1.0.1/` folder to the PC (same version as the Raspi).

```bash
cd ~/micromotion_xyz_v1.0.1
bash Python/setup_venv.sh       # only needed once
source venv/bin/activate
python3 Python/client/xyzKeyboardController.py
```

The client reads `config.toml` (same file) to find the Raspi IP and port.
Open the camera's web interface in your browser while the client is running.

---

## Logging

All motor movements are logged automatically to `Python/client/logs/`.
One log file per day, kept for 30 days. Each entry records:

- Session start / end
- `RUN <axis> <direction>` with timestamp
- `STOP <axis>` with duration the key was held
- Speed changes (Shift+X/Y/Z)
- Emergency stops (ESC)
- Messages received from the firmware (`^XMIN: [RETRACT]`, `^SECURITY`, etc.)

This is the primary tool for diagnosing unexpected overnight motor movement.

---

## v1.0.1 Changes vs Original

| Component      | Change                                                              |
|----------------|---------------------------------------------------------------------|
| Config         | Single `config.toml` replaces scattered hardcoded values           |
| Firmware       | `volatile` added to all ISR-shared flags in `LimitSwitches`        |
| Firmware       | Serial calls removed from ISR (deadlock risk) — deferred to `runAll()` |
| Firmware       | 5 ms debounce on limit switch ISR                                  |
| Firmware       | `minTriggered`/`maxTriggered` now correctly set in ISR             |
| Firmware       | ETX terminator fixed on single-axis `move` response                |
| Firmware       | `Cmd.cpp`: buffer overflow guard, argv off-by-one fix              |
| Firmware       | `Cmd.cpp`: `.` no longer recalls last command (broke decimal args) |
| Firmware       | `ControlService`: per-axis limit handling, removed global stop-all |
| Server         | `SO_REUSEADDR`, sends `stop` on client disconnect                  |
| Client         | Long-press stop bug fixed; speed-toggle debounce; full logging     |
| Layout         | No nested same-name folders; `venv/` at repo root                  |

---

## Credits

System design and functional specifications by Rodrigo J. Gonçalves (Universidad de Granada).
Original firmware implementation, component sourcing and assembly by Ignacio Martínez Navajas (<imnavajas@coit.es>).
Subsequent firmware and control software revisions by Rodrigo J. Gonçalves.

## Status

Research instrumentation under active development. Interfaces and functionalities may change without notice. Issues and pull requests are welcome.

## License

MIT — see [`LICENSE`](LICENSE).

## Citation

If you use this system in your research, please cite the archived release:

> Gonçalves, R. J. (2026). *micromotion_xyz: open-source motorized XYZ
> positioning stage for microscale imaging* (v1.0.1). Zenodo.
> https://doi.org/10.5281/zenodo.21508440

Machine-readable metadata: [`CITATION.cff`](CITATION.cff).

## Funding

This work was developed within the project *Identificación de microorganismos utilizando análisis de movimientos con inteligencia artificial* (CNS2023-143635), funded by MICIU/AEI/10.13039/501100011033 and by the European Union NextGenerationEU/PRTR.

## Contact

Rodrigo J. Gonçalves
Universidad de Granada
rogo@ugr.es
[ORCID](https://orcid.org/0000-0002-4332-2312)


---
