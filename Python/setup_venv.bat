@echo off
setlocal

echo ============================================
echo   xyzTableServer - Virtual Environment Setup
echo ============================================

REM Step 1: Create the virtual environment
echo.
echo Creating virtual environment in 'venv'...
python -m venv venv
if errorlevel 1 (
    echo ❌ Failed to create virtual environment.
    exit /b 1
)

REM Step 2: Activate it
echo.
echo Activating virtual environment...
call venv\Scripts\activate
if errorlevel 1 (
    echo ❌ Failed to activate virtual environment.
    exit /b 1
)

REM Step 3: Install dependencies
echo.
echo Installing required packages...
pip install --upgrade pip
pip install -r requirements.txt
if errorlevel 1 (
    echo ❌ Dependency installation failed.
    exit /b 1
)

REM Success message
echo.
echo ✅ Environment setup complete!
echo.
echo To activate the environment later, run:
echo     venv\Scripts\activate
echo.

endlocal

