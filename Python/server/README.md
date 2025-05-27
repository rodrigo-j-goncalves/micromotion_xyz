# XYZ Table Server

This is the **server component** of the XYZ Table Positioning System. It acts as a relay between TCP clients and the Arduino-based XYZ controller via a serial interface.

---

## ⚙️ Features

- Listens for incoming TCP socket connections
- Parses and forwards commands to the Arduino over USB serial
- Logs all received commands and system responses
- Supports environment-based configuration

---

## 🧩 Dependencies

Install required packages using:

```bash
pip install -r ../requirements.txt
```

Ensure Python 3.7+ is installed.

---

## 🛠️ Configuration

You can set configuration options using a `.env` file:

```
HOST=0.0.0.0
PORT=9000
SERIAL_PORT=/dev/ttyUSB0   # Or COM3 on Windows
BAUD_RATE=115200
```

Alternatively, modify the variables directly in `xyzTableServer.py`.

---

## ▶️ Usage

Start the server by running:

```bash
python xyzTableServer.py
```

The server will:

1. Connect to the Arduino via serial.
2. Accept TCP connections from clients.
3. Forward received commands to the Arduino.
4. Return responses back to the client.
5. Log all activity to `server/logs/xyzTableServer.log`.

---

## 📝 Notes

- Make sure the Arduino board is connected and appears on the configured serial port.
- Use the provided client (`client/xyzTableClient.py`) to test the server.
- Supports multiple clients but handles them sequentially (for now).
- Logs can be used for debugging communication issues or tracking usage.

---