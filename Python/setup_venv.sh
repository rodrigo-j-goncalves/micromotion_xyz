#!/bin/bash

echo "🔧 Setting up virtual environment..."
python3 -m venv venv

echo "📦 Activating and installing dependencies..."
source venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt

echo "✅ Environment ready. To activate it later, run:"
echo "    source venv/bin/activate"
