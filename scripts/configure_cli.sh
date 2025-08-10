#!/bin/bash

CONFIG_FILE="config.mk"

# Function to read a config value
get_config_value() {
    grep "^$1 =" "$CONFIG_FILE" | cut -d'=' -f2 | sed 's/^[[:space:]]*//'
}

# Function to set a config value
set_config_value() {
    local key="$1"
    local value="$2"
    # Use sed to replace the line, or add it if it doesn't exist
    if grep -q "^$key =" "$CONFIG_FILE"; then
        sed -i "s|^$key =.*|$key = $value|" "$CONFIG_FILE"
    else
        echo "$key = $value" >> "$CONFIG_FILE"
    fi
}

# Read current configuration
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

# Ensure default values if not found
DEFAULT_LAUNCHER=${DEFAULT_LAUNCHER:-proton}
BUILD_MODE=${BUILD_MODE:-release}
BUILD_KERNEL=${BUILD_KERNEL:-Y}
BUILD_GUI=${BUILD_GUI:-Y}
ENABLE_ULTRA_LOGGING=${ENABLE_ULTRA_LOGGING:-Y}
USE_WINE=${USE_WINE:-0}
USE_PROTON=${USE_PROTON:-0}
ENABLE_FPS_OVERLAY=${ENABLE_FPS_OVERLAY:-no}
ENABLE_LTO=${ENABLE_LTO:-N}
ENABLE_NATIVE=${ENABLE_NATIVE:-N}

echo "Starting interactive BarrierLayer configuration..."
echo "-------------------------------------------------"

# Function to prompt for Y/N
prompt_yes_no() {
    local var_name="$1"
    local current_value="$2"
    local prompt_text="$3"
    local new_value

    while true; do
        read -p "$prompt_text (Current: $current_value) [Y/N]: " new_value
        new_value=$(echo "$new_value" | tr '[:lower:]' '[:upper:]')
        if [[ "$new_value" == "Y" ]]; then
            eval "$var_name=Y"
            break
        elif [[ "$new_value" == "N" ]]; then
            eval "$var_name=N"
            break
        else
            echo "Invalid input. Please enter Y or N."
        fi
    done
}

# Function to prompt for 0/1
prompt_zero_one() {
    local var_name="$1"
    local current_value="$2"
    local prompt_text="$3"
    local new_value

    while true; do
        read -p "$prompt_text (Current: $current_value) [0/1]: " new_value
        if [[ "$new_value" == "0" || "$new_value" == "1" ]]; then
            eval "$var_name=$new_value"
            break
        else
            echo "Invalid input. Please enter 0 or 1."
        fi
    done
}

# Function to prompt for yes/no (for FPS overlay)
prompt_yes_no_fps() {
    local var_name="$1"
    local current_value="$2"
    local prompt_text="$3"
    local new_value

    while true; do
        read -p "$prompt_text (Current: $current_value) [yes/no]: " new_value
        new_value=$(echo "$new_value" | tr '[:upper:]' '[:lower:]')
        if [[ "$new_value" == "yes" ]]; then
            eval "$var_name=yes"
            break
        elif [[ "$new_value" == "no" ]]; then
            eval "$var_name=no"
            break
        else
            echo "Invalid input. Please enter yes or no."
        fi
    done
}

# --- Configuration Prompts ---

echo ""
echo "--- General Build Options ---"
prompt_yes_no "BUILD_KERNEL" "$BUILD_KERNEL" "Build Kernel Module?"
prompt_yes_no "BUILD_GUI" "$BUILD_GUI" "Build Graphical User Interface (GUI)?"
prompt_yes_no "ENABLE_ULTRA_LOGGING" "$ENABLE_ULTRA_LOGGING" "Enable Ultra Logging (detailed internal logs)?"

echo ""
echo "--- Compatibility Options ---"
prompt_zero_one "USE_WINE" "$USE_WINE" "Enable Wine Support (for Windows applications)?"
prompt_zero_one "USE_PROTON" "$USE_PROTON" "Enable Proton Support (for Steam Play compatibility)?"
prompt_yes_no_fps "ENABLE_FPS_OVERLAY" "$ENABLE_FPS_OVERLAY" "Enable In-Game FPS Overlay (OpenGL only for now)?"

echo ""
echo "--- Advanced Optimization Options ---"
prompt_yes_no "ENABLE_LTO" "$ENABLE_LTO" "Enable Link-Time Optimization (LTO - smaller, faster binaries)?"
prompt_yes_no "ENABLE_NATIVE" "$ENABLE_NATIVE" "Enable Native CPU Optimizations (tailored for your CPU, may not be portable)?"

echo ""
echo "--- Launcher and Build Mode ---"
# DEFAULT_LAUNCHER
while true; do
    read -p "Default Launcher (Current: $DEFAULT_LAUNCHER) [proton/wine/auto]: " new_launcher
    new_launcher=$(echo "$new_launcher" | tr '[:upper:]' '[:lower:]')
    if [[ "$new_launcher" == "proton" || "$new_launcher" == "wine" || "$new_launcher" == "auto" ]]; then
        DEFAULT_LAUNCHER="$new_launcher"
        break
    else
        echo "Invalid input. Please enter proton, wine, or auto."
    fi
done

# BUILD_MODE
while true; do
    read -p "Build Mode (Current: $BUILD_MODE) [release/debug/relwithdebinfo]: " new_build_mode
    new_build_mode=$(echo "$new_build_mode" | tr '[:upper:]' '[:lower:]')
    if [[ "$new_build_mode" == "release" || "$new_build_mode" == "debug" || "$new_build_mode" == "relwithdebinfo" ]]; then
        BUILD_MODE="$new_build_mode"
        break
    else
        echo "Invalid input. Please enter release, debug, or relwithdebinfo."
    fi
done


# --- Write updated configuration ---
echo ""
echo "Updating $CONFIG_FILE with new settings..."

# Overwrite the file with new values
cat > "$CONFIG_FILE" <<EOL
# BarrierLayer Configuration
# This file is automatically generated by 'make configs'. Do not edit manually.

DEFAULT_LAUNCHER = $DEFAULT_LAUNCHER

# Compiler (not configurable via CLI for now)
CC = gcc
AS = nasm

# Build Mode
BUILD_MODE = $BUILD_MODE

# Build Features
BUILD_KERNEL = $BUILD_KERNEL
BUILD_GUI = $BUILD_GUI
ENABLE_ULTRA_LOGGING = $ENABLE_ULTRA_LOGGING

# Compatibility
USE_WINE = $USE_WINE
USE_PROTON = $USE_PROTON

# FPS Overlay
ENABLE_FPS_OVERLAY = $ENABLE_FPS_OVERLAY

# Optimizations
ENABLE_LTO = $ENABLE_LTO
ENABLE_NATIVE = $ENABLE_NATIVE

# Kernel build directory (not configurable via CLI for now)
KDIR ?= /lib/modules/\$(shell uname -r)/build
EOL

echo "-------------------------------------------------"
echo "Configuration complete. Run 'make' to build with new settings."
