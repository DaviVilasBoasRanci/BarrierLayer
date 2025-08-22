# BarrierLayer Advanced Build System
# Version 2.1

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

# Base compiler flags
CFLAGS = -Isrc/include -Wall -Wextra $(CFLAGS_MODE) -D_GNU_SOURCE -fPIC -fstack-protector-strong -D_FORTIFY_SOURCE=2
LDFLAGS = -Wl,-z,relro,-z,now -ldl -lseccomp -pthread

# Diretórios
SRC_DIR = src
BIN_DIR = bin
KERNEL_DIR = kernel
CORE_DIR = $(SRC_DIR)/core
HOOK_DIR = $(SRC_DIR)/hooks

# Arquivos Fonte
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.c)
HHOOK_SOURCES = $(wildcard $(HOOK_DIR)/*.c)
STEALTH_LAUNCHER_SRC = $(SRC_DIR)/stealth_launcher.c
SANDBOX_LAUNCHER_SRC = $(SRC_DIR)/sandbox_launcher.c
SANDBOX_CORE_SRC = $(SRC_DIR)/sandbox/sandbox_core.c

# Arquivos Objeto
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
HHOOK_OBJECTS = $(HHOOK_SOURCES:.c=.o)

# Binários
STEALTH_LAUNCHER_BIN = $(BIN_DIR)/stealth_launcher
SANDBOX_LAUNCHER_BIN = $(BIN_DIR)/sandbox_launcher
HHOOK_LIBRARY = $(BIN_DIR)/barrierlayer_hook.so

# Alvo Padrão
all: $(BIN_DIR) $(HHOOK_LIBRARY) $(STEALTH_LAUNCHER_BIN) $(SANDBOX_LAUNCHER_BIN)
ifeq ($(BUILD_KERNEL),Y)
	$(MAKE) kernel
endif

# Criar diretório de binários
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Regras de Compilação
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(STEALTH_LAUNCHER_BIN): $(STEALTH_LAUNCHER_SRC)
	@echo -e "$(BLUE)[INFO]$(NC) Building Stealth Launcher..."
	$(CC) $(CFLAGS) -pie -o $@ $< $(LDFLAGS)

SANDBOX_LAUNCHER_SRC_MAIN = $(SRC_DIR)/sandbox/main_sandbox_launcher.c

$(SANDBOX_LAUNCHER_BIN): $(SANDBOX_LAUNCHER_SRC_MAIN) $(SANDBOX_CORE_SRC) $(CORE_OBJECTS)
	@echo -e "$(BLUE)[INFO]$(NC) Building Sandbox Launcher..."
	$(CC) $(CFLAGS) -pie -o $@ $(SANDBOX_LAUNCHER_SRC_MAIN) $(SANDBOX_CORE_SRC) $(CORE_OBJECTS) $(LDFLAGS)

$(HHOOK_LIBRARY): $(CORE_OBJECTS) $(HHOOK_OBJECTS)
	@echo -e "$(BLUE)[INFO]$(NC) Building Hook Library..."
	$(CC) $(CFLAGS) -shared -o $@ $(CORE_OBJECTS) $(HHOOK_OBJECTS) $(LDFLAGS)

# Alvo do Kernel
kernel:
	@echo -e "$(BLUE)[INFO]$(NC) Building kernel module..."
	$(MAKE) -C $(KERNEL_DIR) all

# Alvo de Instalação
install:
	@echo -e "$(BLUE)[INFO]$(NC) Installing..."
	sudo cp $(STEALTH_LAUNCHER_BIN) /usr/local/bin/
	sudo cp $(SANDBOX_LAUNCHER_BIN) /usr/local/bin/
	sudo cp $(HHOOK_LIBRARY) /usr/local/lib/
	@echo -e "$(GREEN)[SUCCESS]$(NC) Installation complete."

# Alvo de Limpeza
clean:
	@echo -e "$(BLUE)[INFO]$(NC) Cleaning..."
	rm -rf $(BIN_DIR)
	rm -f $(CORE_DIR)/*.o $(HOOK_DIR)/*.o
	$(MAKE) -C $(KERNEL_DIR) clean
	@echo -e "$(GREEN)[SUCCESS]$(NC) Clean complete."

.PHONY: all clean install kernel