CC = gcc
AS = nasm
LDFLAGS = -ldl -lseccomp -pthread
CFLAGS = -Isrc/include -Wall -Wextra -g -D_GNU_SOURCE -fPIC -no-pie
ASFLAGS = -f elf64

# Diretórios do projeto
SRC_DIR = src
HOOK_DIR = $(SRC_DIR)/hooks
SANDBOX_DIR = $(SRC_DIR)/sandbox
ASM_DIR = assembly
INCLUDE_DIR = $(SRC_DIR)/include
CORE_DIR = $(SRC_DIR)/core
BIN_DIR = bin

# Arquivos fonte
ASM_SOURCES = $(wildcard $(ASM_DIR)/*.asm)
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.c)
HOOK_SOURCES = $(wildcard $(HOOK_DIR)/*.c)
SANDBOX_SOURCES = $(wildcard $(SANDBOX_DIR)/*.c)
GUI_SOURCE = $(SRC_DIR)/barrierlayer_gui.c

# Objetos
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
HOOK_OBJECTS = $(HOOK_SOURCES:.c=.o)
SANDBOX_OBJECTS = $(SANDBOX_SOURCES:.c=.o)

.PHONY: all build build-test test clean install

all: build

# Alvos principais
build: bin/barrierlayer bin/barrierlayer_hook.so bin/barrierlayer_gui

# Compilação do executável principal
bin/barrierlayer: $(CORE_OBJECTS) $(ASM_OBJECTS) $(SANDBOX_OBJECTS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compilação da biblioteca compartilhada de hooks
bin/barrierlayer_hook.so: $(HOOK_OBJECTS) $(ASM_OBJECTS)
	@mkdir -p bin
	$(CC) -shared -fPIC $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compilação da interface gráfica
bin/barrierlayer_gui: $(SRC_DIR)/barrierlayer_gui.c
	@mkdir -p bin
	$(CC) $(CFLAGS) $< -o $@ `pkg-config --cflags --libs gtk+-3.0`

# Regras de compilação
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Instalação
install: build
	@echo "Instalando BarrierLayer..."
	./install_barrierlayer_script.sh
	@echo "Instalação concluída!"

# Limpeza
clean:
	rm -rf bin/*.o bin/barrierlayer bin/barrierlayer_hook.so bin/barrierlayer_gui
	rm -f $(ASM_OBJECTS) $(CORE_OBJECTS) $(HOOK_OBJECTS) $(SANDBOX_OBJECTS)



# Lista de arquivos-fonte para os hooks
HOOK_SOURCES = \
	src/hooks/file_hooks.c \
	src/hooks/process_hooks.c \
	src/hooks/registry_hooks.c \
	src/hooks/system_hooks.c \
	src/hooks/network_hooks.c \
	src/hooks/hardware_hooks.c \
	src/hooks/crypto_hooks.c \
	src/hooks/thread_hooks.c \
	src/hooks/memory_hooks.c \
	src/hooks/kernel_hooks.c \
	src/hooks/service_hooks.c \
	src/hooks/wmi_hooks.c \
	src/hooks/debug_hooks.c

# Compila a biblioteca de hook para ser usada com LD_PRELOAD
bin/barrierlayer_hook.so: $(HOOK_SOURCES) src/core/logger.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -fPIC -shared $(HOOK_SOURCES) src/core/logger.c -o $@ $(LDFLAGS)


# Compila o executável de teste, que chama as funções dinamicamente
bin/test_runner: tests/test_runner.c
	@mkdir -p bin
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Target para rodar os testes (depende da construção dos artefatos)
test:
	@make --no-print-directory build
	@echo "\nExecutando teste com LD_PRELOAD..."
	LD_PRELOAD=./bin/barrierlayer_hook.so ./bin/test_runner

usage:
	@echo ""
	@echo "--- Instruções de Uso Universal (Steam, Epic, etc) ---"
	@echo "1. Compile o projeto com: make build"
	@echo "2. Instale o script no PATH com: ./install_barrierlayer_script.sh"
	@echo "3. Certifique-se de que ~/.local/bin está no seu PATH."
	@echo "4. Use o comando abaixo em qualquer launcher (Steam, Epic, etc):"
	@echo "   ENABLE_BARRIERLAYER=1 run_with_barrierlayer.sh %command%"
	@echo "   ou para Epic: ENABLE_BARRIERLAYER=1 run_with_barrierlayer.sh /caminho/para/JogoEpic.exe"
	@echo ""
	@echo "O log de atividades será salvo em: $(shell pwd)/barrierlayer_activity.log"
	@echo ""