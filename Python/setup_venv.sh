#!/bin/bash
# Creates a virtual environment at the repo root and installs all dependencies.
# Run once from any location:
#   bash Python/setup_venv.sh
#
# To activate afterwards:
#   source venv/bin/activate          (from repo root)
#   source ../../venv/bin/activate    (from Python/client/ or Python/server/)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
VENV_DIR="$REPO_ROOT/venv"
REQUIREMENTS="$SCRIPT_DIR/requirements.txt"

echo "Creating virtual environment at: $VENV_DIR"
python3 -m venv "$VENV_DIR"

echo "Installing dependencies..."
"$VENV_DIR/bin/pip" install --upgrade pip --quiet
"$VENV_DIR/bin/pip" install -r "$REQUIREMENTS"

echo ""
echo "Done. To activate:"
echo "  source $VENV_DIR/bin/activate"
echo ""
echo "To start the server:"
echo "  cd $REPO_ROOT/Python/server && python3 xyzTableServer.py"
echo ""
echo "To start the client:"
echo "  cd $REPO_ROOT/Python/client && python3 xyzKeyboardController.py"
