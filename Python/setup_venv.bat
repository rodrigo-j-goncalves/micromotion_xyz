@echo off
echo Setting up virtual environment...
python -m venv venv

echo Activating and installing dependencies...
call venv\Scripts\activate
pip install --upgrade pip
pip install -r requirements.txt

echo.
echo ✅ Environment ready!
echo To activate it later, run:
echo     venv\Scripts\activate
