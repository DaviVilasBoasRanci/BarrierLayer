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

# 32-bit specific flags
CFLAGS_32 = -Isrc/include -Wall -Wextra $(CFLAGS_MODE) -D_GNU_SOURCE -fPIC -no-pie -m32
LDFLAGS_32 = -ldl -lseccomp -pthread -m32
ASFLAGS_32 = -f elf32

# Ultra logging support
ifeq ($(ENABLE_ULTRA_LOGGING),Y)
    CFLAGS += -DULTRA_LOGGING_ENABLED
endif

# FPS Overlay support
ifeq ($(ENABLE_FPS_OVERLAY),yes)
    CFLAGS += -DENABLE_FPS_OVERLAY
    LDFLAGS += -lGL -lGLX
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
HHOOK_SOURCES = $(wildcard $(HOOK_DIR)/*.c)
SANDBOX_SOURCES = $(wildcard $(SANDBOX_DIR)/*.c)
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.c)
GUI_SOURCES = $(wildcard $(GUI_DIR)/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.asm)

# Objetos
HHOOK_OBJECTS = $(HOOK_SOURCES:.c=.o)
SANDBOX_OBJECTS = $(SANDBOX_SOURCES:.c=.o)
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
GUI_OBJECTS = $(GUI_SOURCES:.c=.o)
HHOOK_OBJECTS = $(HOOK_SOURCES:.c=.o)
SANDBOX_OBJECTS = $(SANDBOX_SOURCES:.c=.o)
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
GUI_OBJECTS = $(GUI_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

# Objetos 32-bit
HHOOK_OBJECTS_32 = $(patsubst %.c,%.o32,$(HHOOK_SOURCES))
SANDBOX_OBJECTS_32 = $(patsubst %.c,%.o32,$(SANDBOX_SOURCES))
CORE_OBJECTS_32 = $(patsubst %.c,%.o32,$(CORE_SOURCES))
ASM_OBJECTS_32 = $(patsubst %.asm,%.o32,$(ASM_SOURCES))

# Executáveis
MAIN_BINARY = $(BIN_DIR)/barrierlayer-cli
HHOOK_LIBRARY = $(BIN_DIR)/barrierlayer_hook.so
HHOOK_LIBRARY_32 = $(BIN_DIR)/barrierlayer_hook32.so
GUI_BINARY = $(BIN_DIR)/barrierlayer_gui
TEST_BINARY = $(BIN_DIR)/test_runner

# Advanced Kernel-Mode Components
STEALTH_LAUNCHER_SRC = src/stealth_launcher.c
ADVANCED_INJECTION_SRC = src/advanced_injection.c
KERNEL_ADVANCED_SRC = kernel/barrierlayer_kernel_advanced.c

STEALTH_LAUNCHER_BIN = $(BIN_DIR)/stealth_launcher
ADVANCED_INJECTION_BIN = $(BIN_DIR)/advanced_injection
KERNEL_ADVANCED_KO = $(KERNEL_DIR)/barrierlayer_kernel_advanced.ko

# Original Kernel module
KERNEL_MODULE = $(KERNEL_DIR)/barrierlayer_minimal.ko

# Targets principais
.PHONY: all clean install uninstall config help userspace kernel gui test hook32 advanced_components
.PHONY: check-deps install-deps status info

# Target padrão
all: config banner check-deps print-build-config userspace advanced_components
ifeq ($(BUILD_KERNEL),Y)
all: kernel
endif
ifeq ($(BUILD_GUI),Y)
all: gui
endif
ifeq ($(or $(USE_WINE),$(USE_PROTON)),1)
all: hook32
endif
all: status

# New target for advanced user-mode components
advanced_components: $(STEALTH_LAUNCHER_BIN) $(ADVANCED_INJECTION_BIN)

$(STEALTH_LAUNCHER_BIN): $(STEALTH_LAUNCHER_SRC) $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building Stealth Launcher: $@..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(ADVANCED_INJECTION_BIN): $(ADVANCED_INJECTION_SRC) $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building Advanced Injection: $@..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Target para o hook de 32-bit
hook32: $(BIN_DIR) $(HHOOK_LIBRARY_32)
	@echo -e "$(GREEN)[SUCCESS]$(NC) 32-bit hook built"

# Biblioteca de hooks 32-bit
$(HHOOK_LIBRARY_32): $(HHOOK_OBJECTS_32) $(CORE_OBJECTS_32) $(ASM_OBJECTS_32) $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building 32-bit hook library: $@..."
	$(CC) $(CFLAGS_32) -shared -o $@ $(HHOOK_OBJECTS_32) $(CORE_OBJECTS_32) $(ASM_OBJECTS_32) $(LDFLAGS_32)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Built: $@"

# Regras de compilação 32-bit
%.o32: %.c
	@echo -e "$(BLUE)[COMPILE]$(NC) Compiling 32-bit $< to $@..."
	$(CC) $(CFLAGS_32) -c $< -o $@

%.o32: %.asm
	@echo -e "$(BLUE)[ASSEMBLE]$(NC) Assembling 32-bit $< to $@..."
	$(AS) $(ASFLAGS_32) $< -o $@

# Banner


# Executáveis
MAIN_BINARY = $(BIN_DIR)/barrierlayer-cli
HHOOK_LIBRARY = $(BIN_DIR)/barrierlayer_hook.so
GUI_BINARY = $(BIN_DIR)/barrierlayer_gui
TEST_BINARY = $(BIN_DIR)/test_runner

# Kernel module
KERNEL_MODULE = $(KERNEL_DIR)/barrierlayer_minimal.ko

# Targets principais
.PHONY: all clean install uninstall config help userspace kernel gui test
.PHONY: check-deps install-deps status info

# Target padrão
all: banner check-deps print-build-config userspace
ifeq ($(BUILD_KERNEL),Y)
all: kernel
endif
ifeq ($(BUILD_GUI),Y)
all: gui
endif
all: status

print-build-config:
	@echo -e "$(CYAN)[CONFIG]$(NC) Current Build Configuration:"
	@echo -e "$(BLUE)  BUILD_MODE:$(NC) $(BUILD_MODE)"
	@echo -e "$(BLUE)  BUILD_KERNEL:$(NC) $(BUILD_KERNEL)"
	@echo -e "$(BLUE)  BUILD_GUI:$(NC) $(BUILD_GUI)"
	@echo -e "$(BLUE)  USE_WINE:$(NC) $(USE_WINE)"
	@echo -e "$(BLUE)  USE_PROTON:$(NC) $(USE_PROTON)"
	@echo -e "$(BLUE)  ENABLE_ULTRA_LOGGING:$(NC) $(ENABLE_ULTRA_LOGGING)"
	@echo -e "$(BLUE)  CFLAGS:$(NC) $(CFLAGS)"
	@echo -e "$(BLUE)  LDFLAGS:$(NC) $(LDFLAGS)"
	@echo -e "$(BLUE)  ASFLAGS:$(NC) $(ASFLAGS)"
	@echo -e "$(CYAN)----------------------------------------------------$(NC)"


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
userspace: $(BIN_DIR) $(MAIN_BINARY) $(HHOOK_LIBRARY) $(TEST_BINARY)
	@echo -e "$(GREEN)[INFO]$(NC) Starting userspace components build..."
	@echo -e "$(GREEN)[SUCCESS]$(NC) Userspace components built"

# Compilar kernel module
kernel:
ifeq ($(BUILD_KERNEL),Y)
	@echo -e "$(BLUE)[INFO]$(NC) Building kernel module... (Advanced)"
	@if [ ! -d "/lib/modules/$(shell uname -r)/build" ]; then \
		echo -e "$(RED)[ERROR]$(NC) Kernel headers not found in /lib/modules/$(shell uname -r)/build"; \
		echo -e "$(RED)[ERROR]$(NC) Install with: sudo apt-get install linux-headers-$(shell uname -r)"; \
		exit 1; \
	fi
	@echo "obj-m := $(notdir $(KERNEL_ADVANCED_KO))" > $(KERNEL_DIR)/Makefile.kernel
	@echo "$(basename $(notdir $(KERNEL_ADVANCED_KO)))-objs := $(basename $(notdir $(KERNEL_ADVANCED_KO))).o" >> $(KERNEL_DIR)/Makefile.kernel
	@echo "EXTRA_CFLAGS += $(KERNEL_CFLAGS)" >> $(KERNEL_DIR)/Makefile.kernel
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/$(KERNEL_DIR) -f $(PWD)/$(KERNEL_DIR)/Makefile.kernel modules
	@rm -f $(KERNEL_DIR)/Makefile.kernel
	@echo -e "$(GREEN)[SUCCESS]$(NC) Kernel module built: $(KERNEL_ADVANCED_KO)"
else
	@echo -e "$(BLUE)[INFO]$(NC) Kernel module build disabled"
endif



# Compilar GUI
gui:
ifeq ($(BUILD_GUI),Y)
	@echo -e "$(BLUE)[INFO]$(NC) Building GUI..."
	$(CC) $(CFLAGS) $(GUI_CFLAGS) -o $(GUI_BINARY) $(GUI_SOURCES) $(GUI_LDFLAGS)
	@echo -e "$(GREEN)[SUCCESS]$(NC) GUI built: $(GUI_BINARY)"
else
	@echo -e "$(BLUE)[INFO]$(NC) GUI build disabled"
endif

# Executável principal
$(MAIN_BINARY): $(SRC_DIR)/main.c $(CORE_OBJECTS) $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building main executable: $@..."
	$(CC) $(CFLAGS) -o $@ $< $(CORE_OBJECTS) $(LDFLAGS)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Built: $@"

# Biblioteca de hooks
$(HHOOK_LIBRARY): $(HHOOK_OBJECTS) $(CORE_OBJECTS) $(ASM_OBJECTS) $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building hook library: $@..."
	$(CC) $(CFLAGS) -shared -o $@ $(HHOOK_OBJECTS) $(CORE_OBJECTS) $(ASM_OBJECTS) $(LDFLAGS)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Built: $@"

# Test runner
$(TEST_BINARY): $(SRC_DIR)/test_runner.c $(BIN_DIR)
	@echo -e "$(BLUE)[INFO]$(NC) Building test runner: $@..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo -e "$(GREEN)[SUCCESS]$(NC) Built: $@"

# Regras de compilação
%.o: %.c
	@echo -e "$(BLUE)[COMPILE]$(NC) Compiling $< to $@..."
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	@echo -e "$(BLUE)[ASSEMBLE]$(NC) Assembling $< to $@..."
	$(AS) $(ASFLAGS) $< -o $@


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
test: $(TEST_BINARY) $(HHOOK_LIBRARY)
	@echo -e "$(BLUE)[TEST]$(NC) Running tests..."
	@LD_PRELOAD=$(HHOOK_LIBRARY) ./$(TEST_BINARY)
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
		echo "🔧 Kernel module (original): $(KERNEL_MODULE)"; \
		ls -la $(KERNEL_MODULE); \
	fi
	@if [ -f $(KERNEL_ADVANCED_KO) ]; then \
		echo "🔧 Kernel module (advanced): $(KERNEL_ADVANCED_KO)"; \
		ls -la $(KERNEL_ADVANCED_KO); \
	fi
	@echo ""
	@echo "🚀 Usage:"
	@echo "  Basic hook:     LD_PRELOAD=./$(HOOK_LIBRARY) <your_app>"
	@echo "  With logging:   BARRIERLAYER_LOG_LEVEL=0 LD_PRELOAD=./$(HOOK_LIBRARY) <your_app>"
	@echo "  GUI interface:  ./$(GUI_BINARY)"
	@echo "  Configuration:  make config"
	@echo "  Advanced Launcher: ./$(STEALTH_LAUNCHER_BIN)"
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
	$(MAKE) install-deps
	@echo -e "$(BLUE)[INSTALL]$(NC) Installing BarrierLayer..."
	@echo -e "$(BLUE)[INSTALL]$(NC) Creating installation directories..."
	sudo mkdir -p /usr/local/bin /usr/local/lib /usr/local/share/barrierlayer
	@echo -e "$(BLUE)[INSTALL]$(NC) Copying main binary: $(MAIN_BINARY) to /usr/local/bin/"
	sudo cp $(MAIN_BINARY) /usr/local/bin/
	@echo -e "$(BLUE)[INSTALL]$(NC) Copying hook library: $(HHOOK_LIBRARY) to /usr/local/lib/"
	sudo cp $(HHOOK_LIBRARY) /usr/local/lib/
ifeq ($(BUILD_GUI),Y)
	@echo -e "$(BLUE)[INSTALL]$(NC) Copying GUI binary: $(GUI_BINARY) to /usr/local/bin/"
	sudo cp $(GUI_BINARY) /usr/local/bin/
endif
	@echo -e "$(BLUE)[INSTALL]$(NC) Copying new advanced binaries..."
	sudo cp $(STEALTH_LAUNCHER_BIN) /usr/local/bin/
	sudo cp $(ADVANCED_INJECTION_BIN) /usr/local/bin/
ifeq ($(BUILD_KERNEL),Y)
	@echo -e "$(BLUE)[INSTALL]$(NC) Copying advanced kernel module: $(KERNEL_ADVANCED_KO) to /lib/modules/$(shell uname -r)/extra/"
	sudo cp $(KERNEL_ADVANCED_KO) /lib/modules/$(shell uname -r)/extra/
	sudo depmod -a
endif
	@echo -e "$(BLUE)[INSTALL]$(NC) Copying documentation to /usr/local/share/barrierlayer/docs/"
	sudo cp -r docs/ /usr/local/share/barrierlayer/ 2>/dev/null || true
	@echo -e "$(BLUE)[INSTALL]$(NC) Setting executable permissions..."
	sudo chmod +x /usr/local/bin/barrierlayer-cli
	sudo chmod +x /usr/local/bin/stealth_launcher
	sudo chmod +x /usr/local/bin/advanced_injection
	@echo -e "$(GREEN)[SUCCESS]$(NC) BarrierLayer installed to /usr/local/"

# Desinstalação
uninstall:
	@echo -e "$(BLUE)[UNINSTALL]$(NC) Removing BarrierLayer..."
	@echo -e "$(BLUE)[UNINSTALL]$(NC) Removing binaries from /usr/local/bin/"
	sudo rm -f /usr/local/bin/barrierlayer-cli
	sudo rm -f /usr/local/bin/stealth_launcher
	sudo rm -f /usr/local/bin/advanced_injection
	@echo -e "$(BLUE)[UNINSTALL]$(NC) Removing hook library from /usr/local/lib/"
	sudo rm -f /usr/local/lib/barrierlayer_hook.so
ifeq ($(BUILD_KERNEL),Y)
	@echo -e "$(BLUE)[UNINSTALL]$(NC) Removing advanced kernel module..."
	sudo rm -f /lib/modules/$(shell uname -r)/extra/$(notdir $(KERNEL_ADVANCED_KO))
	sudo depmod -a
endif
	@echo -e "$(BLUE)[UNINSTALL]$(NC) Removing shared documentation... (if exists)"
	sudo rm -rf /usr/local/share/barrierlayer/
	@echo -e "$(GREEN)[SUCCESS]$(NC) BarrierLayer uninstalled"

# Limpeza
clean:
	@echo -e "$(BLUE)[CLEAN]$(NC) Cleaning build files..."
	rm -f $(HHOOK_OBJECTS) $(SANDBOX_OBJECTS) $(CORE_OBJECTS) $(GUI_OBJECTS) $(ASM_OBJECTS)
	rm -f $(STEALTH_LAUNCHER_BIN) $(ADVANCED_INJECTION_BIN)
	rm -rf $(BIN_DIR)
	$(MAKE) -C $(KERNEL_DIR) clean || true
	@echo -e "$(GREEN)[SUCCESS]$(NC) Clean completed"

# Limpeza completa
distclean: clean
	@echo -e "$(BLUE)[DISTCLEAN]$(NC) Removing configuration..."
	rm -f config.mk src/include/config.h
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
	@echo "  BUILD_KERNEL=Y/N  - Build kernel module (default: Y)"
	@echo "  BUILD_GUI=Y/N     - Build GUI (default: Y)"
	@echo "  USE_WINE=0/1      - Enable Wine support"
	@echo "  USE_PROTON=0/1    - Enable Proton support"
	@echo ""

# Targets especiais
.SECONDARY: $(HOOK_OBJECTS) $(SANDBOX_OBJECTS) $(CORE_OBJECTS) $(GUI_OBJECTS) $(ASM_OBJECTS)
