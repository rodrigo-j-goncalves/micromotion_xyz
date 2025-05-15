# xyzTableServer

**xyzTableServer** is a TCP/IP-to-Serial bridge designed for Arduino Mega.  
It runs continuously, bridging a single TCP client connection to a serial-connected Arduino.

---

## 🚀 Features

- Transparent TCP ↔ Serial communication  
- Continuous loop serving one client at a time  
- Custom defaults via `.env` or command-line  
- Colorful console output with `colorama`  
- Daily rotating logs in `/logs`  
- Auto-listing of available serial ports on error  
- Cross-platform: Windows, Linux, macOS

---

## 📁 Project Structure

```
project_root/
├── client/                 # Placeholder for future client code
├── server/
│   └── xyzTableServer.py   # Main TCP ↔ Serial bridge script
├── .env                    # Default configuration
├── install_deps.py         # Auto-installs required packages
├── requirements.txt        # List of dependencies
├── setup_venv.sh           # Linux/macOS venv setup script
├── setup_venv.bat          # Windows venv setup script
├── logs/                   # Daily-rotated log files
└── README.md               # Project documentation (this file)
```

---

## 🧱 Requirements

- Python 3.6+  
- Internet connection to install packages  

Dependencies listed in `requirements.txt`:
- `pyserial`
- `colorama`
- `python-dotenv`

Install manually:
```bash
pip install -r requirements.txt
```

---

## ⚙️ Setup

To isolate dependencies, use a **virtual environment**.

### 🟢 Automatic setup

**Windows**  
Run the `setup_venv.bat` script in the project root. It will:
1. Create a `venv` folder
2. Activate it
3. Install dependencies

**Linux/macOS**  
Run:
```bash
bash setup_venv.sh
```
The script will do the same steps and show how to activate the environment.

---

### 🛠️ Manual setup

```bash
python -m venv venv
# Activate:
# Windows: venv\Scripts\activate
# Linux/macOS: source venv/bin/activate
pip install -r requirements.txt
```

---

## 🔧 Installing dependencies

Alternatively, use:
```bash
python install_deps.py
```
This checks for missing packages and installs them automatically.

---

## 🔧 Configuration

Edit the `.env` file to override defaults:

```ini
SERIAL_PORT=COM8
BAUDRATE=9600
HOST=0.0.0.0
PORT=5000
```

---

## ▶️ Running the Server

Activate your environment, then:

```bash
python server/xyzTableServer.py
```

You can also pass arguments to override `.env`:

```bash
python server/xyzTableServer.py --serial-port COM3 --baudrate 115200 --host 127.0.0.1 --port 9000
```

The server will log events and data exchanges, rotate logs daily, and continue serving new clients as they connect.

---

## 🪵 Logging

All logs are saved in `logs/xyzTableServer.log` with daily rotation.  
Logs capture INFO, DEBUG (data flows), and ERROR messages.

---

## 📎 License

This project is released under the **MIT License** — free for personal and commercial use.
