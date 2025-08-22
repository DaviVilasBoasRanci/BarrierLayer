#!/bin/bash

# AppArmor Profile Generator for BarrierLayer
# Generates a basic, restrictive profile for a given executable.

set -e

# Cores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

if [[ "$EUID" -ne 0 ]]; then
  log_error "This script must be run as root (sudo) to interact with AppArmor."
  exit 1
fi

if [[ -z "$1" ]]; then
    echo "Usage: $0 <path_to_executable> [profile_name]"
    echo "Example: $0 /path/to/game.exe my_game_profile"
    exit 1
fi

EXECUTABLE_PATH="$(realpath "$1")"
PROFILE_NAME="$2"

if [[ -z "$PROFILE_NAME" ]]; then
    PROFILE_NAME="barrierlayer.$(basename "$EXECUTABLE_PATH")"
fi

# Sanitize profile name
PROFILE_NAME=$(echo "$PROFILE_NAME" | tr -c 'a-zA-Z0-9._-' '_')
PROFILE_PATH="/etc/apparmor.d/$PROFILE_NAME"

log_info "Generating AppArmor profile for: $EXECUTABLE_PATH"
log_info "Profile Name: $PROFILE_NAME"
log_info "Profile Path: $PROFILE_PATH"

# Diretório do executável
EXECUTABLE_DIR="$(dirname "$EXECUTABLE_PATH")"

# Perfil AppArmor
read -r -d '' PROFILE_CONTENT << EOM
#include <tunables/global>

profile $PROFILE_NAME "$EXECUTABLE_PATH" {
  #include <abstractions/base>
  #include <abstractions/consoles>
  #include <abstractions/nameservice>
  #include <abstractions/wine>

  # Deny by default
  deny capability, # Deny all capabilities
  deny network,    # Deny network access by default
  deny mount,      # Deny mount operations

  # Allow execution of the main binary
  "$EXECUTABLE_PATH" mr,

  # Allow read access to the game's own directory
  "$EXECUTABLE_DIR"/** r,

  # Allow read/execute on common system libraries
  /usr/lib{,32,64}/** mr,
  /lib{,32,64}/** mr,

  # Allow write access to a dedicated save folder (example)
  # Users should customize this part
  owner @{HOME}/.local/share/games/$PROFILE_NAME/ w,
  owner @{HOME}/.local/share/games/$PROFILE_NAME/** rw,

  # Allow network access (can be restricted to specific ports)
  network inet stream,
  network inet dgram,

  # Allow read access to Wine/Proton files if they exist
  owner @{HOME}/.wine/** r,
  owner @{HOME}/.steam/steam/steamapps/** r,

  # Deny write access to home directory, except for the save folder
  deny @{HOME}/ rw,
  deny @{HOME}/** rw,

}
EOM

log_info "Writing profile to $PROFILE_PATH..."

echo -e "$PROFILE_CONTENT" > "$PROFILE_PATH"

log_info "Loading profile into the kernel..."
apparmor_parser -r -W "$PROFILE_PATH"

log_info "Profile loaded successfully."
log_info "To enforce this profile, run the application via:"
log_info "aa-exec -p $PROFILE_NAME -- /path/to/launcher your_app"

