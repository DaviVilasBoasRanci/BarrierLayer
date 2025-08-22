#!/bin/bash

# Enable debug tracing to log every command.
set -x

# run_with_barrierlayer.sh
# Final version - Integrates with the advanced C sandbox launcher.

# Exit immediately if a command exits with a non-zero status.
set -e

# 1. Check if BarrierLayer is enabled
if [ "$ENABLE_BARRIERLAYER" != "1" ]; then
    # If not enabled, just execute the command as is.
    echo "[BarrierLayer] Disabled. Running command directly."
    exec "$@"
fi

# 2. Find the project root directory based on the script's location
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(readlink -f "$SCRIPT_DIR/..")"
SANDBOX_LAUNCHER="$PROJECT_ROOT/bin/sandbox_launcher"

# 3. Check if the sandbox launcher exists
if [ ! -x "$SANDBOX_LAUNCHER" ]; then
    echo "[BarrierLayer] ERROR: Sandbox launcher not found or not executable at $SANDBOX_LAUNCHER" >&2
    exit 1
fi

# 4. Argument and Path Detection
if [ -z "$1" ]; then
    echo "[BarrierLayer] ERROR: No command provided to run." >&2
    exit 1
fi

GAME_EXECUTABLE=$(readlink -f "$1")
GAME_DIR=$(dirname "$GAME_EXECUTABLE")

# Detect Proton path from Steam environment variable
# STEAM_COMPAT_TOOL_PATHS is a colon-separated list, we take the first one.
PROTON_INSTALL_PATH="${STEAM_COMPAT_TOOL_PATHS%%:*}"

if [ -z "$PROTON_INSTALL_PATH" ] || [ ! -d "$PROTON_INSTALL_PATH" ]; then
    echo "[BarrierLayer] WARNING: Could not detect Proton path from STEAM_COMPAT_TOOL_PATHS." >&2
    echo "[BarrierLayer] The game might fail if it relies on Proton/Wine." >&2
    # We can still proceed, as it might be a native Linux game.
fi

# 5. Build the final command

# Start with the launcher itself
CMD=("sudo" "$SANDBOX_LAUNCHER" "--verbose")

# Add the bind mount for the game directory
CMD+=("--bind" "$GAME_DIR:/game")

# Add the bind mount for the Proton directory if it was found
if [ -n "$PROTON_INSTALL_PATH" ] && [ -d "$PROTON_INSTALL_PATH" ]; then
    CMD+=("--bind" "$PROTON_INSTALL_PATH:/proton")
fi

# Add the separator and the actual command to be executed
# We need to adjust the executable path to be inside the sandbox
GAME_EXE_BASENAME=$(basename "$GAME_EXECUTABLE")
SANDBOXED_GAME_PATH="/game/$GAME_EXE_BASENAME"

# Shift the original arguments to remove the executable path
ORIGINAL_ARGS=("$@")
shift

CMD+=("--" "$SANDBOXED_GAME_PATH" "$@")

# 6. Display warning and execute
echo "-----------------------------------------------------------------"
_B_GREEN=`tput setaf 2`
_B_YELLOW=`tput setaf 3`
_B_BOLD=`tput bold`
_B_RESET=`tput sgr0`

echo "${_B_BOLD}${_B_GREEN}[BarrierLayer] LAUNCHING GAME IN SANDBOX${_B_RESET}"
echo ""
echo "${_B_YELLOW}NOTE:${_B_RESET} This script uses 'sudo' to start the sandbox."
echo "For a seamless experience on Steam, you may need to configure"
echo "passwordless sudo for the command below."
echo ""
echo "COMMAND:" 
echo "  ${CMD[*]}"
echo "-----------------------------------------------------------------"

# Execute the command
exec "${CMD[@]}"