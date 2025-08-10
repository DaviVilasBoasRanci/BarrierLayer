#!/bin/bash
# Basic configuration for BarrierLayer

# Cores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Configuring BarrierLayer...${NC}"

#--- Seleção do Lançador Padrão ---
echo ""
echo -e "${YELLOW}Please choose your default launch method:${NC}"
echo "1) Proton (Recommended for Steam games)"
echo "2) Wine (For non-Steam games)"
echo "3) Auto-Detect (Launcher will decide)"

read -p "Enter your choice [1-3]: " choice

LAUNCH_METHOD="auto" # Padrão é auto-detect
case $choice in
    1)
        LAUNCH_METHOD="proton"
        echo -e "${GREEN}Default launcher set to: Proton${NC}"
        ;;
    2)
        LAUNCH_METHOD="wine"
        echo -e "${GREEN}Default launcher set to: Wine${NC}"
        ;;
    3)
        LAUNCH_METHOD="auto"
        echo -e "${GREEN}Default launcher set to: Auto-Detect${NC}"
        ;;
    *)
        echo -e "${YELLOW}Invalid choice. Defaulting to Auto-Detect.${NC}"
        LAUNCH_METHOD="auto"
        ;;
esac

#--- Criação do config.mk ---
echo ""
echo "Creating config.mk..."
cat > config.mk << EOL
# BarrierLayer Configuration

# Default launch method (proton, wine, auto)
DEFAULT_LAUNCHER = ${LAUNCH_METHOD}

# Compiler
CC = gcc

# Flags
CFLAGS = -Isrc/include -Wall -Wextra -g

# Linker Flags
LDFLAGS = -ldl -lX11

# Kernel build directory
KDIR ?= /lib/modules/\\\$(shell uname -r)/build
EOL

echo ""
echo -e "${GREEN}Configuration complete!${NC}"
echo "You can now run 'make' to build the project."
echo "To change settings, run this script again."
