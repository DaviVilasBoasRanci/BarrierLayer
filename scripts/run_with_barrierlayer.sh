# Garante execução sob bash
[ -z "$BASH_VERSION" ] && exec bash "$0" "$@"

#!/bin/bash

# 1. Encontra o caminho absoluto para o diretório do projeto BarrierLayer
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/.."

# Variáveis para armazenar caminhos completos
DETECTED_WINE_FULL_PATH=""
DETECTED_PROTON_FULL_PATH=""

# Função para detectar o caminho completo do Wine
detect_wine_full_path() {
    if command -v wine &> /dev/null; then
        DETECTED_WINE_FULL_PATH=$(which wine)
        return 0
    fi
    local wine_paths=(
        "/usr/bin/wine"
        "/usr/local/bin/wine"
        "$HOME/.local/bin/wine"
        "/opt/wine/bin/wine"
    )
    for path in "${wine_paths[@]}"; do
        if [[ -x "$path" ]]; then
            DETECTED_WINE_FULL_PATH="$path"
            return 0
        fi
    done
    return 1
}

# Função para detectar o caminho completo do Proton
detect_proton_full_path() {
    local proton_paths=(
        "$HOME/.steam/steam/steamapps/common/Proton*"
        "$HOME/.local/share/Steam/steamapps/common/Proton*"
        "/usr/share/steam/steamapps/common/Proton*"
    )
    for pattern in "${proton_paths[@]}"; do
        for path in $pattern; do
            if [[ -d "$path" && -f "$path/proton" ]]; then
                DETECTED_PROTON_FULL_PATH="$path/proton"
                return 0
            fi
        done
    done
    return 1
}

# 3. Verifica a variável de ambiente e age de acordo

LOG_FILE="$PROJECT_ROOT/barrierlayer_gui.txt"
exec > >(tee -a "$LOG_FILE") 2>&1

if [ "$ENABLE_BARRIERLAYER" = "1" ]; then
    echo "[BarrierLayer] Ativado. Iniciando via sandbox_launcher..."
    
    # Construct the sandbox_launcher command
    SANDBOX_LAUNCHER_CMD="$PROJECT_ROOT/bin/sandbox_launcher"
    
    # Check if sandbox_launcher exists and is executable
    if [[ ! -f "$SANDBOX_LAUNCHER_CMD" || ! -x "$SANDBOX_LAUNCHER_CMD" ]]; then
        echo "[BarrierLayer] ERRO: sandbox_launcher não encontrado ou não executável em $SANDBOX_LAUNCHER_CMD"
        exit 1
    fi
    
    # Execute sandbox_launcher with the correct options and pass the game command
    exec "$SANDBOX_LAUNCHER_CMD" --enable-sandbox --verbose -- "$@"
else
    echo "[BarrierLayer] Desativado. Iniciando o jogo sem modificações."
    # If BarrierLayer is disabled, just execute the target game directly
    # This assumes the first argument ($1) is the game executable
    exec "$@"
fi

# 6. Determina qual lançador usar (Wine ou Proton) com base na configuração
CONFIG_FILE="$PROJECT_ROOT/config.mk"
LAUNCHER_CMD="" # Variável para armazenar o comando completo do lançador

# Tenta detectar Wine e Proton
detect_wine_full_path
detect_proton_full_path

# Padrão é wine, mas tenta usar o caminho completo se detectado
if [ -n "$DETECTED_WINE_FULL_PATH" ]; then
    LAUNCHER_CMD="$DETECTED_WINE_FULL_PATH"
    echo "[BarrierLayer] Wine detectado em: $LAUNCHER_CMD"
else
    echo "[BarrierLayer] Aviso: Wine não encontrado no PATH ou locais comuns."
fi

if [ -f "$CONFIG_FILE" ]; then
    # Lê a configuração do DEFAULT_LAUNCHER do config.mk
    LAUNCHER_FROM_CONFIG=$(grep -E '^\s*DEFAULT_LAUNCHER\s*=' "$CONFIG_FILE" | cut -d'=' -f2 | tr -d '[:space:]')
    if [ "$LAUNCHER_FROM_CONFIG" = "proton" ]; then
        if [ -n "$DETECTED_PROTON_FULL_PATH" ]; then
            LAUNCHER_CMD="$DETECTED_PROTON_FULL_PATH"
            echo "[BarrierLayer] Proton configurado como padrão e detectado em: $LAUNCHER_CMD"
        else
            echo "[BarrierLayer] Aviso: Proton configurado como padrão, mas não encontrado. Usando Wine (se disponível)."
        fi
    fi
fi

if [ -z "$LAUNCHER_CMD" ]; then
    echo "[BarrierLayer] ERRO: Nenhum lançador (Wine ou Proton) disponível. Não é possível iniciar o jogo."
    exit 1
fi

echo "[BarrierLayer] Usando o lançador: $LAUNCHER_CMD"

# 7. Executa o jogo com o lançador determinado
# O `exec` substitui o processo do script pelo processo do jogo.
# O comando 'file' pode ter problemas com caminhos que contêm espaços, então vamos tentar uma abordagem mais robusta.
if [ -f "$LAUNCHER_CMD" ]; then
    ESCAPED_LAUNCHER_CMD=$(printf %q "$LAUNCHER_CMD")
    echo "[BarrierLayer] DEBUG: $(file -- $ESCAPED_LAUNCHER_CMD)"
else
    echo "[BarrierLayer] DEBUG: Lançador não encontrado em: $LAUNCHER_CMD"
fi

if [[ "$LAUNCHER_CMD" == *proton ]]; then
    exec "$LAUNCHER_CMD" run "$@"
else
    exec "$LAUNCHER_CMD" "$@"
fi



