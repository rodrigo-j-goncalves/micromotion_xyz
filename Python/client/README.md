# XYZ Table Client

This is the **client component** of the XYZ Table Positioning System. It connects to the server (via TCP) and sends control commands for the stepper motors or system state.

---

## 🚀 Features

- Sends CLI-style commands over a network connection to the server.
- Controls movement and configuration of the XYZ platform.
- Simple and scriptable interface for command testing or automation.

---

## 🧩 Dependencies

Install required packages using:

```bash
pip install -r ../requirements.txt
```

Make sure you have Python 3.7+ installed.

---

## 🛠️ Configuration

The following configuration values may be defined in a `.env` file:

```
HOST=127.0.0.1
PORT=9000
```

Alternatively, edit these values directly in `xyzTableClient.py`.

---

## ▶️ Usage

Run the client from the terminal:

```bash
python xyzTableClient.py
```

You will be prompted to enter commands. Example commands include:

- `move all 10`
- `run x`
- `stop all`
- `version`

These commands are interpreted by the Arduino-based controller via the server.

---

## 📝 Notes

- The server must be running and accessible before launching the client.
- Commands mimic the CLI interface used over direct serial.
- Logging and error handling can be extended for robust integration.

---