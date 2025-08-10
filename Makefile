# BarrierLayer Advanced Build System
# Supports Wine, Proton, Kernel Mode, and Anti-Cheat Protection
# Version: 2.0

# Include configuration if available
-include config.mk

# Default configuration
CC ?= gcc
AS ?= nasm
BUILD_MODE ?= release
BUILD_KERNEL ?= Y
BUILD_GUI ?= Y
ENABLE_ULTRA_LOGGING ?= Y
USE_WINE ?= 0
USE_PROTON ?= 0

# Colors for output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
PURPLE = \033[0;35m
CYAN = \033[0;36m
NC = \033[0m

# Build mode flags
ifeq ($(BUILD_MODE),debug)
    CFLAGS_MODE = -g -O0 -DDEBUG
else ifeq ($(BUILD_MODE),relwithdebinfo)
    CFLAGS_MODE = -g -O2 -DNDEBUG
else
    CFLAGS_MODE = -O2 -DNDEBUG
endif

# Base compiler flags
CFLAGS = -Isrc/include -Wall -Wextra $(CFLAGS_MODE) -D_GNU_SOURCE -fPIC -no-pie
LDFLAGS = -ldl -lseccomp -pthread
ASFLAGS = -f elf64

# Ultra logging support
ifeq ($(ENABLE_ULTRA_LOGGING),Y)
    CFLAGS += -DULTRA_LOGGING_ENABLED
endif

# Wine support
ifeq ($(USE_WINE),1)
    CFLAGS += -DUSE_WINE=1
endif

# Proton support
ifeq ($(USE_PROTON),1)
    CFLAGS += -DUSE_PROTON=1
endif

# GUI support
ifeq ($(BUILD_GUI),Y)
    GUI_CFLAGS = $(shell pkg-config --cflags gtk+-3.0 2>/dev/null || echo "")
    GUI_LDFLAGS = $(shell pkg-config --libs gtk+-3.0 2>/dev/null || echo "")
endif

# LTO support
ifeq ($(ENABLE_LTO),Y)
    CFLAGS += -flto
    LDFLAGS += -flto
endif

# Native optimizations
ifeq ($(ENABLE_NATIVE),Y)
    CFLAGS += -march=native -mtune=native
endif

# Diretórios do projeto
SRC_DIR = src
HOOK_DIR = $(SRC_DIR)/hooks
SANDBOX_DIR = $(SRC_DIR)/sandbox
CORE_DIR = $(SRC_DIR)/core
GUI_DIR = $(SRC_DIR)/gui
BIN_DIR = bin
KERNEL_DIR = kernel

# Arquivos fonte
HOOK_SOURCES = $(wildcard $(HOOK_DIR)/*.c)
SANDBOX_SOURCES = $(wildcard $(SANDBOX_DIR)/*.c)
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.c)
GUI_SOURCES = $(wildcard $(GUI_DIR)/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.asm)

# Objetos
HOOK_OBJECTS = $(HOOK_SOURCES:.c=.o)
SANDBOX_OBJECTS = $(SANDBOX_SOURCES:.c=.o)
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
GUI_OBJECTS = $(GUI_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

# Executáveis
MAIN_BINARY = $(BIN_DIR)/barrierlayer
HOOK_LIBRARY = $(BIN_DIR)/barrierlayer_hook.so
GUI_BINARY = $(BIN_DIR)/barrierlayer_gui
TEST_BINARY = $(BIN_DIR)/test_runner

# Kernel module
KERNEL_MODULE = $(KERNEL_DIR)/barrierlayer_minimal.ko

# Targets principais
.PHONY: all clean install uninstall config help userspace kernel gui test
.PHONY: check-deps install-deps status info

# Target padrão
all: banner check-deps userspace
ifeq ($(BUILD_KERNEL),Y)
all: kernel
endif
ifeq ($(BUILD_GUI),Y)
all: gui
endif
all: status

# Banner
banner:
	@echo -e "$(CYAN)"
	@echo "╔══════════════════════════════════════════════════════════════╗"
	@echo "║                    BarrierLayer v2.0                        ║"
	@echo "║              Advanced Build System                          ║"
	@echo "║                                                              ║"
	@echo "║  Building: $(BUILD_MODE) mode                                      ║"
	@echo "║  Features: Wine=$(USE_WINE) Proton=$(USE_PROTON) Kernel=$(BUILD_KERNEL) GUI=$(BUILD_GUI)        ║"
	@echo "╚══════════════════════════════════════════════════════════════╝"
	@echo -e "$(NC)"

# Verificar dependências
check-deps:
	@echo -e "$(BLUE)[INFO]$(NC) Checking dependencies..."
	@which gcc >/dev/null 2>&1 || (echo -e "$(RED)[ERROR]$(NC) gcc not found" && exit 1)
	@which nasm >/dev/null 2>&1 || (echo -e "$(RED)[ERROR]$(NC) nasm not found" && exit 1)
	@pkg-config --exists libseccomp || (echo -e "$(RED)[ERROR]$(NC) libseccomp-dev not found" && exit 1)
ifeq ($(BUILD_GUI),Y)
	@pkg-config --exists gtk+-3.0 || (echo -e "$(RED)[ERROR]$(NC) libgtk-3-dev not found" && exit 1)
endif
ifeq ($(BUILD_KERNEL),Y)
	@test -d /lib/modules/$(shell uname -r)/build || (echo -e "$(RED)[ERROR]$(NC) Kernel headers not found" && exit 1)
endif
	@echo -e "$(GREEN)[SUCCESS]$(NC) All dependencies satisfied"

# Instalar dependências automaticamente
install-deps:
	@echo -e "$(BLUE)[INFO]$(NC) Installing dependencies..."
	@if command -v apt-get >/dev/null 2>&1; then \
		sudo apt-get update && \
		sudo apt-get install -y gcc nasm libseccomp-dev libgtk-3-dev linux-headers-$(shell uname -r); \
	elif command -v yum >/dev/null 2>&1; then \
		sudo yum install -y gcc nasm libseccomp-devel gtk3-devel kernel-devel; \
	elif command -v pacman >/dev/null 2>&1; then \
		sudo pacman -S gcc nasm libseccomp gtk3 linux-headers; \
	else \
		echo -e "$(RED)[ERROR]$(NC) Unsupported package manager"; \
		exit 1; \
	fi
	@echo -e "$(GREEN)[SUCCESS]$(NC) Dependencies installed"

# Criar diretórios
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Compilar userspace
userspace: $(BIN_DIR) $(MAIN_BINARY) $(HOOK_LIBRARY) $(TEST_BINARY)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Userspace components built"

# Compilar kernel module
kernel:
ifeq ($(BUILD_KERNEL),Y)
	@echo -e "$(BLUE)[INFO]$(NC) Building kernel module..."
	@$(MAKE) -C $(KERNEL_DIR) all
	@if [ -f $(KERNEL_MODULE) ]; then \
		echo -e "$(GREEN)[SUCCESS]$(NC) Kernel module built: $(KERNEL_MODULE)"; \
	else \
		echo -e "$(YELLOW)[WARNING]$(NC) Kernel module build failed (this is optional)"; \
	fi
else
	@echo -e "$(BLUE)[INFO]$(NC) Kernel module build disabled"
endif

# Compilar GUI
gui:
ifeq ($(BUILD_GUI),Y)
	@echo -e "$(BLUE)[INFO]$(NC) Building GUI..."
	@$(CC) $(CFLAGS) $(GUI_CFLAGS) -o $(GUI_BINARY) $(GUI_SOURCES) $(GUI_LDFLAGS)
	@echo -e "$(GREEN)[SUCCESS]$(NC) GUI built: $(GUI_BINARY)"
else
	@echo -e "$(BLUE)[INFO]$(NC) GUI build disabled"
endif

# Executável principal
$(MAIN_BINARY): $(SRC_DIR)/main.c $(CORE_OBJECTS) $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building main executable..."
	@$(CC) $(CFLAGS) -o $@ $< $(CORE_OBJECTS) $(LDFLAGS)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Built: $@"

# Biblioteca de hooks
$(HOOK_LIBRARY): $(HOOK_OBJECTS) $(CORE_OBJECTS) $(ASM_OBJECTS) $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building hook library..."
	@$(CC) $(CFLAGS) -shared -o $@ $(HOOK_OBJECTS) $(CORE_OBJECTS) $(ASM_OBJECTS) $(LDFLAGS)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Built: $@"

# Test runner
$(TEST_BINARY): $(SRC_DIR)/test_runner.c $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building test runner..."
	@$(CC) $(CFLAGS) -o $@ $< $(filter %/kernel_hooks.o,$(HOOK_OBJECTS)) $(LDFLAGS)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Built: $@"

# Regras de compilação
%.o: %.c
	@echo -e "$(BLUE)[COMPILE]$(NC) $<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	@echo -e "$(BLUE)[ASSEMBLE]$(NC) $<"
	@$(AS) $(ASFLAGS) $< -o $@

# Configuração interativa
config:
	@echo -e "$(CYAN)[CONFIG]$(NC) Starting interactive configuration..."
	@./configure.sh

# Configuração CLI amigável
configs:
	@echo -e "$(CYAN)[CONFIG]$(NC) Starting interactive CLI configuration..."
	@$(SHELL) scripts/configure_cli.sh

# Configuração automática
auto-config:
	@echo -e "$(CYAN)[CONFIG]$(NC) Using automatic configuration..."
	@./configure.sh --auto

# Teste
test: $(TEST_BINARY) $(HOOK_LIBRARY)
	@echo -e "$(BLUE)[TEST]$(NC) Running tests..."
	@LD_PRELOAD=./$(HOOK_LIBRARY) ./$(TEST_BINARY)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Tests completed"

# Status do build
status:
	@echo -e "$(CYAN)"
	@echo "╔══════════════════════════════════════════════════════════════╗"
	@echo "║                    Build Status                              ║"
	@echo "╚══════════════════════════════════════════════════════════════╝"
	@echo -e "$(NC)"
	@echo -e "$(BLUE)[INFO]$(NC) Build completed successfully!"
	@echo ""
	@echo "📁 Built files:"
	@ls -la $(BIN_DIR)/ 2>/dev/null || echo "  No files in bin/"
	@if [ -f $(KERNEL_MODULE) ]; then \
		echo "🔧 Kernel module: $(KERNEL_MODULE)"; \
		ls -la $(KERNEL_MODULE); \
	fi
	@echo ""
	@echo "🚀 Usage:"
	@echo "  Standalone:     ./barrierlayer-launcher.sh -p <your_app.exe>"
	@echo "  Steam:          Set game Launch Options to the following:"
	@echo "                  ENABLE_BARRIERLAYER=1 $(shell pwd)/scripts/run_with_barrierlayer.sh %command%"
	@echo "  Log Monitoring: tail -f barrierlayer_gui.log"
	@echo ""

# Informações do sistema
info:
	@echo -e "$(CYAN)System Information:$(NC)"
	@echo "  OS: $(shell lsb_release -d 2>/dev/null | cut -f2 || uname -s)"
	@echo "  Kernel: $(shell uname -r)"
	@echo "  Architecture: $(shell uname -m)"
	@echo "  Compiler: $(shell $(CC) --version | head -1)"
	@echo ""
	@echo -e "$(CYAN)Build Configuration:$(NC)"
	@echo "  Build Mode: $(BUILD_MODE)"
	@echo "  Wine Support: $(USE_WINE)"
	@echo "  Proton Support: $(USE_PROTON)"
	@echo "  Kernel Module: $(BUILD_KERNEL)"
	@echo "  GUI: $(BUILD_GUI)"
	@echo "  Ultra Logging: $(ENABLE_ULTRA_LOGGING)"
	@echo ""

# Instalação
install: all
	@echo -e "$(BLUE)[INSTALL]$(NC) Installing BarrierLayer..."
	@sudo mkdir -p /usr/local/bin /usr/local/lib /usr/local/share/barrierlayer
	@sudo cp $(MAIN_BINARY) /usr/local/bin/
	@sudo cp $(HOOK_LIBRARY) /usr/local/lib/
ifeq ($(BUILD_GUI),Y)
	@sudo cp $(GUI_BINARY) /usr/local/bin/
endif
	@sudo cp -r docs/ /usr/local/share/barrierlayer/ 2>/dev/null || true
	@sudo chmod +x /usr/local/bin/barrierlayer*
	@echo -e "$(GREEN)[SUCCESS]$(NC) BarrierLayer installed to /usr/local/"

# Desinstalação
uninstall:
	@echo -e "$(BLUE)[UNINSTALL]$(NC) Removing BarrierLayer..."
	@sudo rm -f /usr/local/bin/barrierlayer*
	@sudo rm -f /usr/local/lib/barrierlayer_hook.so
	@sudo rm -rf /usr/local/share/barrierlayer/
	@echo -e "$(GREEN)[SUCCESS]$(NC) BarrierLayer uninstalled"

# Limpeza
clean:
	@echo -e "$(BLUE)[CLEAN]$(NC) Cleaning build files..."
	@rm -f $(HOOK_OBJECTS) $(SANDBOX_OBJECTS) $(CORE_OBJECTS) $(GUI_OBJECTS) $(ASM_OBJECTS)
	@rm -rf $(BIN_DIR)
	@$(MAKE) -C $(KERNEL_DIR) clean 2>/dev/null || true
	@echo -e "$(GREEN)[SUCCESS]$(NC) Clean completed"

# Limpeza completa
distclean: clean
	@echo -e "$(BLUE)[DISTCLEAN]$(NC) Removing configuration..."
	@rm -f config.mk src/include/config.h
	@echo -e "$(GREEN)[SUCCESS]$(NC) Distribution clean completed"

# Help
help:
	@echo -e "$(CYAN)BarrierLayer Build System$(NC)"
	@echo ""
	@echo -e "$(YELLOW)Main targets:$(NC)"
	@echo "  all          - Build everything (default)"
	@echo "  userspace    - Build userspace components only"
	@echo "  kernel       - Build kernel module only"
	@echo "  gui          - Build GUI only"
	@echo "  clean        - Remove build files"
	@echo "  distclean    - Remove build files and configuration"
	@echo ""
	@echo -e "$(YELLOW)Configuration:$(NC)"
	@echo "  config       - Interactive configuration"
	@echo "  auto-config  - Automatic configuration"
	@echo "  info         - Show system and build information"
	@echo ""
	@echo -e "$(YELLOW)Installation:$(NC)"
	@echo "  install      - Install to /usr/local/"
	@echo "  uninstall    - Remove from /usr/local/"
	@echo ""
	@echo -e "$(YELLOW)Testing:$(NC)"
	@echo "  test         - Run tests"
	@echo "  check-deps   - Check dependencies"
	@echo "  install-deps - Install dependencies automatically"
	@echo ""
	@echo -e "$(YELLOW)Build modes:$(NC)"
	@echo "  make BUILD_MODE=debug     - Debug build"
	@echo "  make BUILD_MODE=release   - Release build (default)"
	@echo "  make BUILD_MODE=relwithdebinfo - Optimized with debug info"
	@echo ""
	@echo -e "$(YELLOW)Options:$(NC)"
	@echo "  BUILD_KERNEL=Y/N  - Build kernel module (default: Y)"
	@echo "  BUILD_GUI=Y/N     - Build GUI (default: Y)"
	@echo "  USE_WINE=0/1      - Enable Wine support"
	@echo "  USE_PROTON=0/1    - Enable Proton support"
	@echo ""

# Targets especiais
.SECONDARY: $(HOOK_OBJECTS) $(SANDBOX_OBJECTS) $(CORE_OBJECTS) $(GUI_OBJECTS) $(ASM_OBJECTS)