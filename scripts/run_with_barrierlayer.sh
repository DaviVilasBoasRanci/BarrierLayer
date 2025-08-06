[#] Garante execução sob bash
[ -z "$BASH_VERSION" ] && exec bash "$0" "$@"

#!/bin/bash

# 1. Encontra o caminho absoluto para o diretório do projeto BarrierLayer
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/.."

# Função para detectar arquitetura do .exe
detect_arch() {
    file "$1" | grep -q "PE32+" && echo "64" && return
    file "$1" | grep -q "PE32" && echo "32" && return
    echo "unknown"
}

# Função para selecionar o hook correto
select_hook() {
    arch=$(detect_arch "$1")
    if [ "$arch" = "64" ]; then
        echo "$PROJECT_ROOT/bin/barrierlayer_hook.so" # 64-bit
    elif [ "$arch" = "32" ]; then
        echo "$PROJECT_ROOT/bin/barrierlayer_hook32.so" # 32-bit
    else
        echo "$PROJECT_ROOT/bin/barrierlayer_hook.so"
    fi
}

HOOK_SO_PATH=$(select_hook "$1")
echo "[BarrierLayer] Arquitetura detectada: $(detect_arch "$1")"
echo "[BarrierLayer] Hook selecionado: $HOOK_SO_PATH"

# 3. Verifica a variável de ambiente e age de acordo

LOG_FILE="$PROJECT_ROOT/barrierlayer_gui.log"
exec > >(tee -a "$LOG_FILE") 2>&1

if [ "$ENABLE_BARRIERLAYER" = "1" ]; then
    echo "[BarrierLayer] Ativado. Carregando hook: $HOOK_SO_PATH"
    rm -f "$PROJECT_ROOT/barrierlayer_activity.log"
    export LD_PRELOAD="$HOOK_SO_PATH${LD_PRELOAD:+:$LD_PRELOAD}"
    echo "[BarrierLayer] Variáveis de ambiente prontas. Iniciando o jogo..."
else
    echo "[BarrierLayer] Desativado. Iniciando o jogo sem modificações."
fi

# Executa o .exe via wine
check_wine_or_proton() {
    if command -v wine >/dev/null 2>&1; then
        echo "[BarrierLayer] Wine encontrado."
        return 0
    fi
    if command -v proton >/dev/null 2>&1; then
        echo "[BarrierLayer] Proton encontrado."
        return 0
    fi
    echo "[BarrierLayer] Wine/Proton não encontrado. Instalando Wine..."
    if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get update && sudo apt-get install -y wine
    elif command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y wine
    elif command -v pacman >/dev/null 2>&1; then
        sudo pacman -Sy wine
    else
        echo "[BarrierLayer] Não foi possível instalar Wine automaticamente. Instale manualmente."
        exit 1
    fi
}

check_wine_or_proton
wine "$@"
