# XYZ Table — Technician Guide

This guide is for anyone who needs to operate the XYZ motorized positioning table
from their own computer. You do not need to touch the Raspberry Pi or the Arduino —
those are already set up and running in the lab.

---

## What the system does

The XYZ table moves a camera over a sample (aquarium with plankton) using three
motorized axes. You control it with your keyboard while watching the camera feed
in your browser.

**Axis convention** (always from the camera's point of view):

| Axis | Direction             |
|------|-----------------------|
| X    | depth / focus         |
| Y    | left / right          |
| Z    | up / down             |

You are never watching the physical hardware — limit switches stop the motors
automatically if a mechanical boundary is reached.

---

## Requirements

- A computer connected to the **university network** (cable or eduroam WiFi)
- **Python 3.11 or newer** (`python3 --version` to check)
- Linux, macOS, or Windows

---

## One-time setup

### 1. Get the client files

Copy the following two items from the lab PC or shared drive to your computer:

- The folder `micromotion_xyz_v1.0.0/`

Place it anywhere convenient, e.g. `~/micromotion_xyz_v1.0.0/`.

### 2. Create the Python environment

Open a terminal, go to the folder, and run:

```bash
cd ~/micromotion_xyz_v1.0.0
bash Python/setup_venv.sh
```

This installs all dependencies into a local `venv/` folder. It only needs to be
done once.

### 3. Check the server address

Open `config.toml` in a text editor and confirm:

```toml
[network]
host = "172.25.18.77"
port = 5000
```

This is the Raspberry Pi's address on the university network. It should already
be correct — only change it if the Raspi was assigned a new IP.

---

## Starting the client

Every session:

```bash
cd ~/micromotion_xyz_v1.0.0
source venv/bin/activate
python3 Python/client/xyzKeyboardController.py
```

On Windows, replace `source venv/bin/activate` with `venv\Scripts\activate`.

You should see a connection banner. Open the camera's web interface in your
browser at the same time.

---

## Keyboard controls

| Key        | Action                                        |
|------------|-----------------------------------------------|
| ↑          | X+ — focus in (camera approaches sample)      |
| ↓          | X− — focus out                                |
| ←          | Y+ — camera moves left                        |
| →          | Y− — camera moves right                        |
| `a`        | Z− — camera moves up                          |
| `z`        | Z+ — camera moves down                        |
| Hold key   | Motor runs continuously until you release     |
| Shift + X  | Cycle X axis speed (slow → fast → slow …)    |
| Shift + Y  | Cycle Y axis speed                            |
| Shift + Z  | Cycle Z axis speed                            |
| ESC        | Emergency stop — all axes immediately         |
| Q          | Quit the client (also stops all motors)       |

**Tips:**
- Start at slow speed for focusing; switch to fast for large repositioning moves.
- You can press two keys simultaneously for diagonal movement (e.g. ← and `z`).
- If you see a red `[RETRACT]` message, a limit switch was triggered — the motor
  stopped and moved back automatically. The other axes are unaffected.

---

## Speed control

Each axis has two speeds (slow and fast), configurable in `config.toml`:

```toml
[speeds]
x = [200, 800]   # slow, fast  (steps/second)
y = [300, 600]
z = [300, 600]
```

Shift + axis letter cycles through the list. The current speed is shown in the
status line at the top of the client.

---

## Logs

All movements are logged automatically to `Python/client/logs/` (one file per day,
kept for 30 days). This is useful for reviewing what happened during a session or
diagnosing unexpected motor activity.

---

## Troubleshooting

| Problem | What to check |
|---------|---------------|
| `Connection refused` or timeout | Are you on the university network? Is the Raspi on? Ping `172.25.18.77`. |
| Client starts but keys do nothing | Check the client terminal for error messages. Try ESC then a movement key. |
| Red `[RETRACT]` message | Normal — limit switch triggered. Move the axis in the opposite direction. |
| Motor moves but camera doesn't follow | The camera is a separate system; check its web UI independently. |
| `No module named ...` error | Run `bash Python/setup_venv.sh` again, then re-activate the venv. |

---

## Server and Arduino (for reference — normally untouched)

The Raspberry Pi (`172.25.18.77`) runs the server automatically at startup as a
systemd service. You should not need to manage it. If you suspect the server is
down, ask the responsible researcher to check via SSH:

```bash
ssh xyztable@172.25.18.77          # password on request
sudo systemctl status xyztableserver
sudo systemctl restart xyztableserver
```

The Arduino firmware version can be verified by connecting to the Raspi and
opening a serial monitor at 115200 baud, then typing `version`.

---

## Contact

For hardware issues or if the Raspi needs reconfiguration, contact the researcher
responsible for the micropositioning setup.
