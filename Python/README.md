# xyzTableServer

**xyzTableServer** is a TCP/IP-to-Serial bridge designed to interface with an Arduino Mega.  
It continuously bridges a single TCP client connection to a serial-connected Arduino running a CLI-based firmware.

---

## 🚀 Features

- Transparent TCP ↔ Serial communication  
- Continuous loop serving one client at a time  
- Customizable defaults via `.env` or command-line arguments  
- Colorized console output with `colorama`  
- Daily rotating logs stored in `/logs`  
- Auto-detection of available serial ports on failure  
- Cross-platform support: Windows, Linux, macOS

---

## 📁 Project Structure

```
project_root/
├── client/                 # Python client script to send commands
│   ├── xyzTableClient.py
│   └── README.md
├── server/
│   ├── xyzTableServer.py   # Main TCP ↔ Serial bridge script
│   ├── logs/
│   │   └── xyzTableServer.log
│   └── README.md
├── .env                    # Optional environment config (global fallback)
├── install_deps.py         # Installs required packages
├── requirements.txt        # List of Python dependencies
├── setup_venv.sh           # Unix/macOS virtual environment setup
├── setup_venv.bat          # Windows virtual environment setup
└── README.md               # This file
```

---

## 🧱 Requirements

- Python **3.6+**
- Internet connection (to install dependencies)

### 📦 Dependencies

Listed in `requirements.txt`:
- `pyserial`
- `colorama`
- `python-dotenv`

Install manually:

```bash
pip install -r requirements.txt
```

---

## ⚙️ Setup

It is recommended to use a **virtual environment** to isolate dependencies.

### ✅ Automatic Setup

#### On Windows:
```bash
setup_venv.bat
```

#### On Linux/macOS:
```bash
bash setup_venv.sh
```

These scripts will:
1. Create a `venv` folder
2. Activate it
3. Install dependencies

---

### 🔧 Manual Setup

```bash
python -m venv venv

# Activate the virtual environment:
# Windows:
venv\Scripts\activate
# macOS/Linux:
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt
```

---

## 📥 Installing Dependencies (Alternative)

You may also run the bundled installer script:

```bash
python install_deps.py
```

This checks and installs any missing packages automatically.

---

## 🛠️ Configuration

You can define default settings in a `.env` file at the project root, or inside `/server` and `/client` separately:

```ini
SERIAL_PORT=COM8
BAUDRATE=9600
HOST=0.0.0.0
PORT=5000
```

Values from CLI arguments override `.env` settings.

---

## ▶️ Running the Server

Activate your environment and run:

```bash
python server/xyzTableServer.py
```

Override defaults via CLI:

```bash
python server/xyzTableServer.py --serial-port COM3 --baudrate 115200 --host 127.0.0.1 --port 9000
```

---

## ▶️ Running the Client

In a separate terminal:

```bash
python client/xyzTableClient.py
```

Type commands like:

- `move all 10`
- `run x`
- `stop all`
- `version`

---

## 🪵 Logging

Logs are saved in `server/logs/xyzTableServer.log` with daily rotation.  
Log levels include INFO, DEBUG (data), and ERROR.

---

## 👨‍💻 Author

**Ignacio Martínez Navajas**  
📧 imnavajas@coit.es

---

## 📄 License

This project is for educational and research purposes. Please credit the original author if reused.