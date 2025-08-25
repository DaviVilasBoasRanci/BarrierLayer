# BarrierLayer Advanced Build System
# Version 2.2 (Multi-Arch)

# Include configuration if available
-include config.mk

# Default configuration
CC ?= gcc
AS ?= nasm
BUILD_MODE ?= release
BUILD_KERNEL ?= Y
BUILD_GUI ?= Y

# Colors for output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
NC = \033[0m

# Build mode flags
ifeq ($(BUILD_MODE),debug)
    CFLAGS_MODE = -g -O0 -DDEBUG
else
    CFLAGS_MODE = -O2 -DNDEBUG
endif

# --- Base Flags (DO NOT EDIT HERE) ---
BASE_CFLAGS = -Isrc/include -Wall -Wextra $(CFLAGS_MODE) -D_GNU_SOURCE -fPIC -fstack-protector-strong -D_FORTIFY_SOURCE=2
BASE_LDFLAGS = -Wl,-z,relro,-z,now -ldl -lseccomp -pthread

# --- Architecture Specific Flags ---
# 64-bit (default)
CFLAGS64 = $(BASE_CFLAGS)
LDFLAGS64 = $(LDFLAGS64)

# 32-bit
CFLAGS32 = $(BASE_CFLAGS) -m32
LDFLAGS32 = $(LDFLAGS32) -m32

# --- Directories ---
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
OBJ_DIR64 = $(OBJ_DIR)/x64
OBJ_DIR32 = $(OBJ_DIR)/x32
KERNEL_DIR = kernel
CORE_DIR = $(SRC_DIR)/core
HOOK_DIR = $(SRC_DIR)/hooks

# --- Source Files ---
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.c)
HOOK_SOURCES = $(wildcard $(HOOK_DIR)/*.c)
STEALTH_LAUNCHER_SRC = $(SRC_DIR)/stealth_launcher.c
SANDBOX_LAUNCHER_SRC = $(SRC_DIR)/sandbox_launcher.c
SANDBOX_CORE_SRC = $(SRC_DIR)/sandbox/sandbox_core.c
SANDBOX_LAUNCHER_SRC_MAIN = $(SRC_DIR)/sandbox/main_sandbox_launcher.c

# --- Object Files ---
# 64-bit Objects
CORE_OBJECTS64 = $(patsubst $(CORE_DIR)/%.c,$(OBJ_DIR64)/core/%.o,$(CORE_SOURCES))
HOOK_OBJECTS64 = $(patsubst $(HOOK_DIR)/%.c,$(OBJ_DIR64)/hooks/%.o,$(HOOK_SOURCES))
SANDBOX_CORE_OBJ64 = $(patsubst $(SRC_DIR)/sandbox/%.c,$(OBJ_DIR64)/sandbox/%.o,$(SANDBOX_CORE_SRC))

# 32-bit Objects
CORE_OBJECTS32 = $(patsubst $(CORE_DIR)/%.c,$(OBJ_DIR32)/core/%.o,$(CORE_SOURCES))
Hook_OBJECTS32 = $(patsubst $(HOOK_DIR)/%.c,$(OBJ_DIR32)/hooks/%.o,$(HOOK_SOURCES))
SANDBOX_CORE_OBJ32 = $(patsubst $(SRC_DIR)/sandbox/%.c,$(OBJ_DIR32)/sandbox/%.o,$(SANDBOX_CORE_SRC))


# --- Binaries ---
STEALTH_LAUNCHER_BIN = $(BIN_DIR)/stealth_launcher
SANDBOX_LAUNCHER_BIN = $(BIN_DIR)/sandbox_launcher
HOOK_LIBRARY32 = $(BIN_DIR)/barrierlayer_hook32.so
HOOK_LIBRARY64 = $(BIN_DIR)/barrierlayer_hook64.so

# --- Targets ---

# Default Target: Build all essential components for both architectures
all: $(BIN_DIR) $(HOOK_LIBRARY64) $(HOOK_LIBRARY32) $(STEALTH_LAUNCHER_BIN) $(SANDBOX_LAUNCHER_BIN)
ifeq ($(BUILD_KERNEL),Y)
	$(MAKE) kernel
endif

# Create output directories
$(BIN_DIR) $(OBJ_DIR64) $(OBJ_DIR32):
	@mkdir -p $(OBJ_DIR64)/core $(OBJ_DIR64)/hooks $(OBJ_DIR64)/sandbox
	@mkdir -p $(OBJ_DIR32)/core $(OBJ_DIR32)/hooks $(OBJ_DIR32)/sandbox
	@mkdir -p $(BIN_DIR)


# --- Compilation Rules ---

$(OBJ_DIR64)/core/%.o: $(CORE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS64) -c $< -o $@

$(OBJ_DIR64)/hooks/%.o: $(HOOK_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS64) -c $< -o $@

# Specific rules for GDI and User32 hooks (64-bit) to include MinGW headers
$(OBJ_DIR64)/hooks/gdi_hooks.o: $(HOOK_DIR)/gdi_hooks.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS64) -I/usr/x86_64-w64-mingw32/include/ -c $< -o $@

$(OBJ_DIR64)/hooks/user32_hooks.o: $(HOOK_DIR)/user32_hooks.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS64) -I/usr/x86_64-w64-mingw32/include/ -c $< -o $@

$(OBJ_DIR64)/sandbox/%.o: $(SRC_DIR)/sandbox/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS64) -c $< -o $@

$(OBJ_DIR32)/core/%.o: $(CORE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS32) -c $< -o $@

$(OBJ_DIR32)/hooks/%.o: $(HOOK_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS32) -c $< -o $@

# Specific rules for GDI and User32 hooks (32-bit) to include MinGW headers
$(OBJ_DIR32)/hooks/gdi_hooks.o: $(HOOK_DIR)/gdi_hooks.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS32) -I/usr/i686-w64-mingw32/include/ -c $< -o $@

$(OBJ_DIR32)/hooks/user32_hooks.o: $(HOOK_DIR)/user32_hooks.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS32) -I/usr/i686-w64-mingw32/include/ -c $< -o $@

$(OBJ_DIR32)/sandbox/%.o: $(SRC_DIR)/sandbox/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS32) -c $< -o $@


# --- Linking Rules ---

# Build 64-bit Hook Library
$(HOOK_LIBRARY64): $(CORE_OBJECTS64) $(HOOK_OBJECTS64)
	@echo -e "$(BLUE)[INFO]$(NC) Building 64-bit Hook Library..."
	$(CC) $(CFLAGS64) -shared -o $@ $^ $(LDFLAGS64)

# Build 32-bit Hook Library
$(HOOK_LIBRARY32): $(CORE_OBJECTS32) $(HOOK_OBJECTS32)
	@echo -e "$(BLUE)[INFO]$(NC) Building 32-bit Hook Library..."
	$(CC) $(CFLAGS32) -shared -o $@ $^ $(LDFLAGS32)

# Build Launchers (defaulting to 64-bit for now)
$(STEALTH_LAUNCHER_BIN): $(STEALTH_LAUNCHER_SRC) | $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building Stealth Launcher (64-bit)..."
	$(CC) $(CFLAGS64) -pie -o $@ $< $(LDFLAGS64)

$(SANDBOX_LAUNCHER_BIN): $(SANDBOX_LAUNCHER_SRC_MAIN) $(SANDBOX_CORE_SRC) $(CORE_OBJECTS64) | $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building Sandbox Launcher (64-bit)..."
	$(CC) $(CFLAGS64) -pie -o $@ $(filter %.c,$^) $(filter %.o,$^) $(LDFLAGS64)


# --- Other Targets ---

# Alvo do Kernel
kernel:
	@echo -e "$(BLUE)[INFO]$(NC) Building kernel module..."
	$(MAKE) -C $(KERNEL_DIR) all

# Alvo de Instalação
install:
	@echo -e "$(BLUE)[INFO]$(NC) Installing..."
	sudo cp $(STEALTH_LAUNCHER_BIN) /usr/local/bin/
	sudo cp $(SANDBOX_LAUNCHER_BIN) /usr/local/bin/
	sudo cp $(HOOK_LIBRARY64) /usr/local/lib/barrierlayer64.so
	sudo cp $(HOOK_LIBRARY32) /usr/local/lib/barrierlayer32.so
	@echo -e "$(GREEN)[SUCCESS]$(NC) Installation complete."

# Alvo de Limpeza
clean:
	@echo -e "$(BLUE)[INFO]$(NC) Cleaning..."
	rm -rf $(BIN_DIR) $(OBJ_DIR)
	$(MAKE) -C $(KERNEL_DIR) clean
	@echo -e "$(GREEN)[SUCCESS]$(NC) Clean complete."

.PHONY: all clean install kernel