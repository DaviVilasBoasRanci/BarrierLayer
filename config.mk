# BarrierLayer Configuration

# Default launch method (proton, wine, auto)
DEFAULT_LAUNCHER = proton

# Compiler
CC = gcc

# Flags
CFLAGS = -Isrc/include -Wall -Wextra -g

# Linker Flags
LDFLAGS = -ldl -lX11

# Kernel build directory
KDIR ?= /lib/modules/\$(shell uname -r)/build
