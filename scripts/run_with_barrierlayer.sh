#!/bin/bash

# run_with_barrierlayer.sh
# Uses bwrap to create a sandbox and LD_PRELOAD to inject the anti-cheat hook library.

# --- Configuration ---
# Find the project root directory based on the script's location
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(readlink -f "$SCRIPT_DIR/../")"
FAKE_C_DRIVE="$PROJECT_ROOT/fake_c_drive"
VIRTUAL_DESKTOP="$FAKE_C_DRIVE/drive_c/users/root/Desktop"

# Path to the BarrierLayer hook libraries on the host
BARRIERLAYER_BIN_DIR="$PROJECT_ROOT/bin"
BARRIERLAYER_HOOK32_SO="$BARRIERLAYER_BIN_DIR/barrierlayer_hook32.so"
BARRIERLAYER_HOOK64_SO="$BARRIERLAYER_BIN_DIR/barrierlayer_hook64.so"

# --- Pre-flight Checks ---
# Check if an executable path was provided
if [ -z "$1" ]; then
    echo "[BarrierLayer ERROR] No executable path provided."
    echo "Usage: $0 /path/to/your/game.exe"
    exit 1
fi

HOST_EXE_PATH="$1"
if [ ! -f "$HOST_EXE_PATH" ]; then
    echo "[BarrierLayer ERROR] File not found: $HOST_EXE_PATH"
    exit 1
fi

# Check for the hook libraries
if [ ! -f "$BARRIERLAYER_HOOK32_SO" ] || [ ! -f "$BARRIERLAYER_HOOK64_SO" ]; then
    echo "[BarrierLayer ERROR] Hook libraries not found. Missing:"
    [ ! -f "$BARRIERLAYER_HOOK32_SO" ] && echo "  - $BARRIERLAYER_HOOK32_SO"
    [ ! -f "$BARRIERLAYER_HOOK64_SO" ] && echo "  - $BARRIERLAYER_HOOK64_SO"
    echo "Please compile the project with 'make all' first."
    exit 1
fi

# --- Execution ---
echo "[BarrierLayer] Starting application: $HOST_EXE_PATH"

# 1. Copy the executable to the virtual desktop inside our fake C: drive
echo "[BarrierLayer] Copying executable into the Wine prefix..."
# Ensure the destination directory exists
mkdir -p "$VIRTUAL_DESKTOP" 
cp "$HOST_EXE_PATH" "$VIRTUAL_DESKTOP/"
if [ $? -ne 0 ]; then
    echo "[BarrierLayer ERROR] Failed to copy executable into the Wine prefix."
    exit 1
fi

# 2. Prepare paths for Wine
EXE_FILENAME=$(basename "$HOST_EXE_PATH")
# Wine requires backslashes
SANDBOX_EXE_PATH="C:\\users\\root\\Desktop\\$EXE_FILENAME"

echo "[BarrierLayer] Executable is now at: $SANDBOX_EXE_PATH"
echo "[BarrierLayer] Injecting anti-cheat hooks..."
echo "[BarrierLayer] Launching..."
echo "-----------------------------------------------------------------"

# 3. Run the executable directly with Wine (without bwrap/firejail sandbox)
WINEPREFIX="$FAKE_C_DRIVE" \
DISPLAY="$DISPLAY" \
LD_PRELOAD="/barrierlayer_bin/barrierlayer_hook64.so:/barrierlayer_bin/barrierlayer_hook32.so" \
wine "$SANDBOX_EXE_PATH"

EXIT_CODE=$?
echo "-----------------------------------------------------------------"
echo "[BarrierLayer] Process exited with code: $EXIT_CODE"

# 4. Clean up the copied executable
echo "[BarrierLayer] Cleaning up..."
rm "$VIRTUAL_DESKTOP/$EXE_FILENAME"

exit $EXIT_CODE
