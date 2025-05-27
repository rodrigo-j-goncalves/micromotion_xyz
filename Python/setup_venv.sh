#!/bin/bash

echo "============================================"
echo "  xyzTableServer - Virtual Environment Setup"
echo "============================================"

# Step 1: Check for Python
if ! command -v python3 &> /dev/null
then
    echo "❌ Python 3 is not installed or not in PATH."
    exit 1
fi

# Step 2: Create virtual environment
echo "🔧 Creating virtual environment in ./venv..."
python3 -m venv venv
if [ $? -ne 0 ]; then
    echo "❌ Failed to create virtual environment."
    exit 1
fi

# Step 3: Activate and install dependencies
echo "📦 Activating virtual environment and installing dependencies..."
source venv/bin/activate

pip install --upgrade pip
if [ $? -ne 0 ]; then
    echo "❌ Failed to upgrade pip."
    exit 1
fi

pip install -r requirements.txt
if [ $? -ne 0 ]; then
    echo "❌ Failed to install dependencies."
    exit 1
fi

# Success message
echo ""
echo "✅ Environment setup complete!"
echo ""
echo "To activate the environment later, run:"
echo "    source venv/bin/activate"
echo ""
