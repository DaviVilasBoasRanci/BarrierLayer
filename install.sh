#!/bin/bash

# Installation script for the BarrierLayer command

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
SOURCE_FILE="$SCRIPT_DIR/barrierlayer"
DEST_DIR="/usr/local/bin"
DEST_FILE="$DEST_DIR/barrierlayer"

echo "This script will install the 'barrierlayer' command to your system."

# Check if the source file exists
if [ ! -f "$SOURCE_FILE" ]; then
    echo "ERROR: The source file was not found at $SOURCE_FILE"
    echo "Please run this script from the project's root directory."
    exit 1
fi

# Check if the destination directory exists and is in the PATH
if ! [[ ":$PATH:" == *":$DEST_DIR:"* ]]; then
    echo "WARNING: $DEST_DIR is not in your PATH."
    echo "You may not be able to run the 'barrierlayer' command directly after installation."
fi

echo "The command will be installed to $DEST_FILE"
echo "This may require administrative privileges (sudo)."
echo ""

# Make the source file executable first
echo "Making the 'barrierlayer' script executable..."
chmod +x "$SOURCE_FILE"
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to make the script executable."
    exit 1
fi

#!/bin/bash

# Installation script for the bl-run command

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
SOURCE_FILE="$SCRIPT_DIR/bl-run"
DEST_DIR="/usr/local/bin"
DEST_FILE="$DEST_DIR/bl-run"

echo "This script will install the 'bl-run' command to your system."

# Check if the source file exists
if [ ! -f "$SOURCE_FILE" ]; then
    echo "ERROR: The source file was not found at $SOURCE_FILE"
    echo "Please run this script from the project's root directory."
    exit 1
fi

# Check if the destination directory exists and is in the PATH
if ! [[ ":$PATH:" == *":$DEST_DIR:"* ]]; then
    echo "WARNING: $DEST_DIR is not in your PATH."
    echo "You may not be able to run the 'bl-run' command directly after installation."
fi

echo "The command will be installed to $DEST_FILE"
echo "This may require administrative privileges (sudo)."
echo ""

# Make the source file executable first
echo "Making the 'bl-run' script executable..."
chmod +x "$SOURCE_FILE"
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to make the script executable."
    exit 1
fi

# Copy the file using sudo
if sudo cp "$SOURCE_FILE" "$DEST_FILE"; then
    echo ""
    echo "'bl-run' command installed successfully!"
    echo "You can now run your applications using:"
    echo "  bl-run /path/to/your/game.exe"
else
    echo ""
    echo "ERROR: Failed to install the command."
    echo "Please try running this script with sudo: sudo ./install.sh"
    exit 1
fi

exit 0

exit 0