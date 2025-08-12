#!/bin/bash
# Interactive configuration for BarrierLayer using dialog

# Cores para mensagens do script
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Cores para o dialog (usando sequências de escape \Z)
# Veja man dialog para mais opções de cores
DLG_RED="\Z1"
DLG_GREEN="\Z2"
DLG_YELLOW="\Z3"
DLG_BLUE="\Z4"
DLG_MAGENTA="\Z5"
DLG_CYAN="\Z6"
DLG_WHITE="\Z7"
DLG_BLACK="\Z0"
DLG_NORMAL="\Z0"

# Check if dialog is installed
if ! command -v dialog &> /dev/null
then
    echo -e "${RED}Error:${NC} 'dialog' command not found."
    echo -e "Please install 'dialog' to use the interactive configuration."
    echo -e "  On Debian/Ubuntu: ${YELLOW}sudo apt-get install dialog${NC}"
    echo -e "  On Fedora/RHEL:   ${YELLOW}sudo yum install dialog${NC}"
    echo -e "  On Arch Linux:    ${YELLOW}sudo pacman -S dialog${NC}"
    exit 1
fi

# Temporary file for dialog output
TEMP_FILE=$(mktemp)

# Function to read a config value from config.mk
get_config_value() {
    if [ -f config.mk ]; then
        grep "^$1 =" config.mk | cut -d'=' -f2 | sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//'
    else
        echo ""
    fi
}

# Read current configuration or set defaults
DEFAULT_LAUNCHER=$(get_config_value "DEFAULT_LAUNCHER")
BUILD_MODE=$(get_config_value "BUILD_MODE")
BUILD_KERNEL=$(get_config_value "BUILD_KERNEL")
BUILD_GUI=$(get_config_value "BUILD_GUI")
ENABLE_ULTRA_LOGGING=$(get_config_value "ENABLE_ULTRA_LOGGING")
USE_WINE=$(get_config_value "USE_WINE")
USE_PROTON=$(get_config_value "USE_PROTON")
ENABLE_FPS_OVERLAY=$(get_config_value "ENABLE_FPS_OVERLAY")
ENABLE_LTO=$(get_config_value "ENABLE_LTO")
ENABLE_NATIVE=$(get_config_value "ENABLE_NATIVE")

# Set default values if not found
DEFAULT_LAUNCHER=${DEFAULT_LAUNCHER:-auto}
BUILD_MODE=${BUILD_MODE:-release}
BUILD_KERNEL=${BUILD_KERNEL:-Y}
BUILD_GUI=${BUILD_GUI:-Y}
ENABLE_ULTRA_LOGGING=${ENABLE_ULTRA_LOGGING:-Y}
USE_WINE=${USE_WINE:-0}
USE_PROTON=${USE_PROTON:-0}
ENABLE_FPS_OVERLAY=${ENABLE_FPS_OVERLAY:-no}
ENABLE_LTO=${ENABLE_LTO:-N}
ENABLE_NATIVE=${ENABLE_NATIVE:-N}

# --- Main Menu ---
while true; do
    dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
           --title "${DLG_YELLOW}Main Configuration Menu${DLG_NORMAL}" \
           --menu "${DLG_CYAN}Choose a configuration category:${DLG_NORMAL}" 18 60 7 \
           "1" "${DLG_GREEN}General Build Options${DLG_NORMAL}" \
           "2" "${DLG_GREEN}Compatibility Options${DLG_NORMAL}" \
           "3" "${DLG_GREEN}Optimization Options${DLG_NORMAL}" \
           "4" "${DLG_GREEN}Launcher and Build Mode${DLG_NORMAL}" \
           "5" "${DLG_RED}Save and Exit${DLG_NORMAL}" \
           "6" "${DLG_RED}Discard Changes and Exit${DLG_NORMAL}" 2> "$TEMP_FILE"

    MENU_CHOICE=$(cat "$TEMP_FILE")
    EXIT_STATUS=$?

    if [ $EXIT_STATUS -ne 0 ] || [ "$MENU_CHOICE" == "6" ]; then
        # User pressed ESC, Cancel, or chose Discard
        dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
               --title "${DLG_RED}Discard Changes?${DLG_NORMAL}" \
               --yesno "${DLG_YELLOW}Are you sure you want to discard all changes and exit?${DLG_NORMAL}" 7 40
        if [ $? -eq 0 ]; then
            # User confirmed discard
            break
        else
            # User cancelled discard, return to main menu
            continue
        fi
    fi

    case "$MENU_CHOICE" in
        1) # General Build Options
            dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
                   --title "${DLG_YELLOW}General Build Options${DLG_NORMAL}" \
                   --radiolist "${DLG_CYAN}Choose BarrierLayer Mode:${DLG_NORMAL}" 10 60 2 \
                   "USER_MODE_ONLY" "User Mode Only (No Kernel Module)" "$([ "$BUILD_KERNEL" == "N" ] && echo "on" || echo "off")" \
                   "USER_KERNEL_MODE" "User Mode + Kernel Mode" "$([ "$BUILD_KERNEL" == "Y" ] && echo "on" || echo "off")" \
                   2> "$TEMP_FILE"
            
            MODE_CHOICE=$(cat "$TEMP_FILE")
            if [ $? -eq 0 ]; then # Check if OK was pressed
                if [ "$MODE_CHOICE" == "USER_MODE_ONLY" ]; then
                    BUILD_KERNEL="N"
                elif [ "$MODE_CHOICE" == "USER_KERNEL_MODE" ]; then
                    BUILD_KERNEL="Y"
                fi
            fi

            dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
                   --title "${DLG_YELLOW}General Build Options${DLG_NORMAL}" \
                   --checklist "${DLG_CYAN}Select other general features:${DLG_NORMAL}" 15 60 5 \
                   "BUILD_GUI" "Build Graphical User Interface (GUI)" "$([ "$BUILD_GUI" == "Y" ] && echo "on" || echo "off")" \
                   "ENABLE_ULTRA_LOGGING" "Enable Ultra Logging (detailed internal logs)" "$([ "$ENABLE_ULTRA_LOGGING" == "Y" ] && echo "on" || echo "off")" \
                   2> "$TEMP_FILE"
            
            GENERAL_OPTS=$(cat "$TEMP_FILE")
            if [ $? -eq 0 ]; then # Check if OK was pressed
                BUILD_GUI=$(echo "$GENERAL_OPTS" | grep -q "BUILD_GUI" && echo "Y" || echo "N")
                ENABLE_ULTRA_LOGGING=$(echo "$GENERAL_OPTS" | grep -q "ENABLE_ULTRA_LOGGING" && echo "Y" || echo "N")
            fi
            ;;
        2) # Compatibility Options
            dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
                   --title "${DLG_YELLOW}Compatibility Options${DLG_NORMAL}" \
                   --checklist "${DLG_CYAN}Select compatibility features:${DLG_NORMAL}" 15 60 5 \
                   "USE_WINE" "Enable Wine Support (for Windows applications)" "$([ "$USE_WINE" == "1" ] && echo "on" || echo "off")" \
                   "USE_PROTON" "Enable Proton Support (for Steam Play compatibility)" "$([ "$USE_PROTON" == "1" ] && echo "on" || echo "off")" \
                   "ENABLE_FPS_OVERLAY" "Enable In-Game FPS Overlay (OpenGL only for now)" "$([ "$ENABLE_FPS_OVERLAY" == "yes" ] && echo "on" || echo "off")" \
                   2> "$TEMP_FILE"
            
            COMPAT_OPTS=$(cat "$TEMP_FILE")
            if [ $? -eq 0 ]; then # Check if OK was pressed
                USE_WINE=$(echo "$COMPAT_OPTS" | grep -q "USE_WINE" && echo "1" || echo "0")
                USE_PROTON=$(echo "$COMPAT_OPTS" | grep -q "USE_PROTON" && echo "1" || echo "0")
                ENABLE_FPS_OVERLAY=$(echo "$COMPAT_OPTS" | grep -q "ENABLE_FPS_OVERLAY" && echo "yes" || echo "no")
            fi
            ;;
        3) # Optimization Options
            dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
                   --title "${DLG_YELLOW}Optimization Options${DLG_NORMAL}" \
                   --checklist "${DLG_CYAN}Select optimization features:${DLG_NORMAL}" 15 60 5 \
                   "ENABLE_LTO" "Enable Link-Time Optimization (LTO - smaller, faster binaries)" "$([ "$ENABLE_LTO" == "Y" ] && echo "on" || echo "off")" \
                   "ENABLE_NATIVE" "Enable Native CPU Optimizations (tailored for your CPU, may not be portable)" "$([ "$ENABLE_NATIVE" == "Y" ] && echo "on" || echo "off")" \
                   2> "$TEMP_FILE"
            
            OPT_OPTS=$(cat "$TEMP_FILE")
            if [ $? -eq 0 ]; then # Check if OK was pressed
                ENABLE_LTO=$(echo "$OPT_OPTS" | grep -q "ENABLE_LTO" && echo "Y" || echo "N")
                ENABLE_NATIVE=$(echo "$OPT_OPTS" | grep -q "ENABLE_NATIVE" && echo "Y" || echo "N")
            fi
            ;;
        4) # Launcher and Build Mode
            dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
                   --title "${DLG_YELLOW}Launcher and Build Mode${DLG_NORMAL}" \
                   --radiolist "${DLG_CYAN}Choose default launch method:${DLG_NORMAL}" 15 60 3 \
                   "proton" "Proton (Recommended for Steam games)" "$([ "$DEFAULT_LAUNCHER" == "proton" ] && echo "on" || echo "off")" \
                   "wine" "Wine (For non-Steam games)" "$([ "$DEFAULT_LAUNCHER" == "wine" ] && echo "on" || echo "off")" \
                   "auto" "Auto-Detect (Launcher will decide)" "$([ "$DEFAULT_LAUNCHER" == "auto" ] && echo "on" || echo "off")" \
                   2> "$TEMP_FILE"
            
            LAUNCHER_CHOICE=$(cat "$TEMP_FILE")
            if [ $? -eq 0 ]; then # Check if OK was pressed
                DEFAULT_LAUNCHER="$LAUNCHER_CHOICE"
            fi

            dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
                   --title "${DLG_YELLOW}Launcher and Build Mode${DLG_NORMAL}" \
                   --radiolist "${DLG_CYAN}Choose Build Mode:${DLG_NORMAL}" 15 60 3 \
                   "release" "Release (Optimized, default)" "$([ "$BUILD_MODE" == "release" ] && echo "on" || echo "off")" \
                   "debug" "Debug (Debugging symbols, no optimization)" "$([ "$BUILD_MODE" == "debug" ] && echo "on" || echo "off")" \
                   "relwithdebinfo" "Release with Debug Info (Optimized with debugging symbols)" "$([ "$BUILD_MODE" == "relwithdebinfo" ] && echo "on" || echo "off")" \
                   2> "$TEMP_FILE"
            
            BUILD_MODE_CHOICE=$(cat "$TEMP_FILE")
            if [ $? -eq 0 ]; then # Check if OK was pressed
                BUILD_MODE="$BUILD_MODE_CHOICE"
            fi
            ;;
        5) # Save and Exit
            dialog --clear --colors --backtitle "${DLG_BLUE}BarrierLayer Configuration${DLG_NORMAL}" \
                   --title "${DLG_GREEN}Save Configuration?${DLG_NORMAL}" \
                   --yesno "${DLG_YELLOW}Are you sure you want to save changes and exit?${DLG_NORMAL}" 7 40
            if [ $? -eq 0 ]; then
                # User confirmed save
                break
            else
                # User cancelled save, return to main menu
                continue
            fi
            ;;
    esac
done

# --- Write updated configuration to config.mk ---
echo ""
echo "Creating config.mk..."
cat > config.mk << EOL
# BarrierLayer Configuration
# This file is automatically generated by 'make config'. Do not edit manually.

# Default launch method (proton, wine, auto)
DEFAULT_LAUNCHER = ${DEFAULT_LAUNCHER}

# Build Mode
BUILD_MODE = ${BUILD_MODE}

# Build Features
BUILD_KERNEL = ${BUILD_KERNEL}
BUILD_GUI = ${BUILD_GUI}
ENABLE_ULTRA_LOGGING = ${ENABLE_ULTRA_LOGGING}

# Compatibility
USE_WINE = ${USE_WINE}
USE_PROTON = ${USE_PROTON}

# FPS Overlay
ENABLE_FPS_OVERLAY = ${ENABLE_FPS_OVERLAY}

# Optimizations
ENABLE_LTO = ${ENABLE_LTO}
ENABLE_NATIVE = ${ENABLE_NATIVE}

# Compiler (not configurable via CLI for now)
CC = gcc
AS = nasm

# Kernel build directory (not configurable via CLI for now)
KDIR ?= /lib/modules/$(shell uname -r)/build
EOL

# Clean up temporary file
rm -f "$TEMP_FILE"

echo ""
echo -e "${GREEN}Configuration complete!${NC}"
echo "You can now run 'make' to build the project."
echo "To change settings, run 'make config' again."