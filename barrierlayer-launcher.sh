#!/bin/bash

# BarrierLayer Smart Launcher
# Automatically detects and configures Wine/Proton environment
# Version: 2.0

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

# Configurações
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HOOK_LIBRARY="$SCRIPT_DIR/bin/barrierlayer_hook.so"
CONFIG_FILE="$SCRIPT_DIR/config.mk"

# Variáveis globais
DETECTED_WINE=""
DETECTED_PROTON=""
DETECTED_STEAM=""
TARGET_EXECUTABLE=""
LAUNCH_MODE=""
VERBOSE=0

# Função para log
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_debug() {
    if [[ $VERBOSE -eq 1 ]]; then
        echo -e "${PURPLE}[DEBUG]${NC} $1"
    fi
}

# Banner
show_banner() {
    echo -e "${CYAN}"
    cat << "EOF"
╔══════════════════════════════════════════════════════════════╗
║                 BarrierLayer Launcher v2.0                  ║
║              Smart Wine/Proton Detection                    ║
║                                                              ║
║  🍷 Wine Support    🚂 Proton Support    🛡️ Anti-Cheat      ║
╚══════════════════════════════════════════════════════════════╝
EOF
    echo -e "${NC}"
}

# Detectar Wine
detect_wine() {
    log_debug "Detecting Wine installations..."
    
    # Verificar Wine no PATH
    if command -v wine &> /dev/null; then
        DETECTED_WINE=$(which wine)
        local wine_version=$(wine --version 2>/dev/null | head -1)
        log_success "Wine found: $wine_version at $DETECTED_WINE"
        return 0
    fi
    
    # Verificar localizações comuns
    local wine_paths=(
        "/usr/bin/wine"
        "/usr/local/bin/wine"
        "$HOME/.local/bin/wine"
        "/opt/wine/bin/wine"
    )
    
    for path in "${wine_paths[@]}"; do
        if [[ -x "$path" ]]; then
            DETECTED_WINE="$path"
            local wine_version=$($path --version 2>/dev/null | head -1)
            log_success "Wine found: $wine_version at $path"
            return 0
        fi
    done
    
    log_warning "Wine not found"
    return 1
}

# Detectar Proton
detect_proton() {
    log_debug "Detecting Proton installations..."
    
    local proton_paths=(
        "$HOME/.steam/steam/steamapps/common/Proton*"
        "$HOME/.local/share/Steam/steamapps/common/Proton*"
        "/usr/share/steam/steamapps/common/Proton*"
    )
    
    local found_proton=()
    
    for pattern in "${proton_paths[@]}"; do
        for path in $pattern; do
            if [[ -d "$path" && -f "$path/proton" ]]; then
                local version=$(basename "$path")
                found_proton+=("$path:$version")
                log_success "Proton found: $version at $path"
            fi
        done
    done
    
    if [[ ${#found_proton[@]} -gt 0 ]]; then
        # Usar a versão mais recente (última na lista)
        IFS=':' read -r DETECTED_PROTON proton_version <<< "${found_proton[-1]}"
        log_info "Using Proton: $proton_version"
        return 0
    fi
    
    log_warning "Proton not found"
    return 1
}

# Detectar Steam
detect_steam() {
    log_debug "Detecting Steam..."
    
    if command -v steam &> /dev/null; then
        DETECTED_STEAM=$(which steam)
        log_success "Steam found at $DETECTED_STEAM"
        return 0
    fi
    
    log_warning "Steam not found"
    return 1
}

# Detectar ambiente atual
detect_environment() {
    log_info "Detecting current environment..."
    
    # Verificar se já estamos em um ambiente Wine/Proton
    if [[ -n "$WINEPREFIX" ]]; then
        log_info "Already in Wine environment: $WINEPREFIX"
        LAUNCH_MODE="wine-existing"
        return 0
    fi
    
    if [[ -n "$STEAM_COMPAT_DATA_PATH" ]]; then
        log_info "Already in Proton environment: $STEAM_COMPAT_DATA_PATH"
        LAUNCH_MODE="proton-existing"
        return 0
    fi
    
    # Detectar por argumentos ou contexto
    if [[ "$TARGET_EXECUTABLE" == *.exe ]]; then
        log_info "Windows executable detected: $TARGET_EXECUTABLE"
        
        # Preferir Proton se disponível
        if detect_proton; then
            LAUNCH_MODE="proton"
        elif detect_wine; then
            LAUNCH_MODE="wine"
        else
            log_error "No Wine or Proton found for Windows executable"
            return 1
        fi
    else
        log_info "Native Linux executable: $TARGET_EXECUTABLE"
        LAUNCH_MODE="native"
    fi
    
    return 0
}

# Configurar ambiente Wine
setup_wine_environment() {
    log_info "Setting up Wine environment..."
    
    if [[ -z "$DETECTED_WINE" ]]; then
        detect_wine || return 1
    fi
    
    # Configurar WINEPREFIX se não definido
    if [[ -z "$WINEPREFIX" ]]; then
        export WINEPREFIX="$HOME/.wine"
        log_debug "Set WINEPREFIX=$WINEPREFIX"
    fi
    
    # Configurações básicas do Wine
    export WINEARCH="win64"
    export WINEDEBUG="-all"
    export WINE_LARGE_ADDRESS_AWARE="1"
    
    log_debug "Wine environment configured"
}

# Configurar ambiente Proton
setup_proton_environment() {
    log_info "Setting up Proton environment..."
    
    if [[ -z "$DETECTED_PROTON" ]]; then
        detect_proton || return 1
    fi
    
    # Configurar variáveis do Proton
    export WINEPREFIX="${STEAM_COMPAT_DATA_PATH:-$HOME/.steam/steam/steamapps/compatdata/barrierlayer}/pfx"
    export WINEDLLPATH="$DETECTED_PROTON/dist/lib64/wine:$DETECTED_PROTON/dist/lib/wine"
    export PATH="$DETECTED_PROTON/dist/bin:$PATH"
    
    # Configurações do Proton
    export DXVK_HUD="0"
    export DXVK_LOG_LEVEL="none"
    export VKD3D_DEBUG="none"
    export WINEESYNC="1"
    export WINEFSYNC="1"
    
    # Criar prefixo se não existir
    mkdir -p "$WINEPREFIX"
    
    log_debug "Proton environment configured"
}

# Configurar BarrierLayer
setup_barrierlayer() {
    log_info "Setting up BarrierLayer..."
    
    # Verificar se a biblioteca existe
    if [[ ! -f "$HOOK_LIBRARY" ]]; then
        log_error "Hook library not found: $HOOK_LIBRARY"
        log_info "Please run 'make' to build BarrierLayer first"
        return 1
    fi
    
    # Configurar LD_PRELOAD
    if [[ -n "$LD_PRELOAD" ]]; then
        export LD_PRELOAD="$HOOK_LIBRARY:$LD_PRELOAD"
    else
        export LD_PRELOAD="$HOOK_LIBRARY"
    fi
    
    # Configurações de logging
    export BARRIERLAYER_LOG_LEVEL="${BARRIERLAYER_LOG_LEVEL:-2}"
    export BARRIERLAYER_DETAILED="${BARRIERLAYER_DETAILED:-1}"
    export BARRIERLAYER_STACK_TRACE="${BARRIERLAYER_STACK_TRACE:-1}"
    
    # Configurações anti-cheat
    export BARRIERLAYER_STEALTH_MODE="1"
    export BARRIERLAYER_ANTI_CHEAT="1"
    
    log_success "BarrierLayer configured"
}

# Executar aplicação
launch_application() {
    local executable="$1"
    shift
    local args=("$@")
    
    log_info "Launching: $executable ${args[*]}"
    
    case "$LAUNCH_MODE" in
        "native")
            log_info "Launching as native Linux application..."
            exec "$executable" "${args[@]}"
            ;;
        "wine"|"wine-existing")
            setup_wine_environment
            log_info "Launching via Wine..."
            exec "$DETECTED_WINE" "$executable" "${args[@]}"
            ;;
        "proton"|"proton-existing")
            setup_proton_environment
            log_info "Launching via Proton..."
            exec "$DETECTED_PROTON/proton" run "$executable" "${args[@]}"
            ;;
        *)
            log_error "Unknown launch mode: $LAUNCH_MODE"
            return 1
            ;;
    esac
}

# Menu interativo
show_interactive_menu() {
    while true; do
        clear
        show_banner
        
        echo -e "${YELLOW}=== Launch Options ===${NC}"
        echo ""
        echo "Detected environments:"
        [[ -n "$DETECTED_WINE" ]] && echo "  🍷 Wine: $(basename "$DETECTED_WINE")"
        [[ -n "$DETECTED_PROTON" ]] && echo "  🚂 Proton: $(basename "$DETECTED_PROTON")"
        [[ -n "$DETECTED_STEAM" ]] && echo "  🎮 Steam: $(basename "$DETECTED_STEAM")"
        echo ""
        
        echo "1) 🍷 Launch with Wine"
        echo "2) 🚂 Launch with Proton"
        echo "3) 🐧 Launch as native Linux"
        echo "4) 🔍 Auto-detect (recommended)"
        echo "5) ⚙️  Configure BarrierLayer"
        echo "6) 📊 Show system info"
        echo "0) ❌ Exit"
        echo ""
        
        read -p "Choose option [0-6]: " choice
        
        case $choice in
            1)
                if [[ -n "$DETECTED_WINE" ]]; then
                    LAUNCH_MODE="wine"
                    break
                else
                    log_error "Wine not available"
                    read -p "Press Enter to continue..."
                fi
                ;;
            2)
                if [[ -n "$DETECTED_PROTON" ]]; then
                    LAUNCH_MODE="proton"
                    break
                else
                    log_error "Proton not available"
                    read -p "Press Enter to continue..."
                fi
                ;;
            3)
                LAUNCH_MODE="native"
                break
                ;;
            4)
                detect_environment
                break
                ;;
            5)
                cd "$SCRIPT_DIR" && make config
                read -p "Press Enter to continue..."
                ;;
            6)
                show_system_info
                read -p "Press Enter to continue..."
                ;;
            0)
                exit 0
                ;;
            *)
                log_error "Invalid option!"
                sleep 1
                ;;
        esac
    done
}

# Mostrar informações do sistema
show_system_info() {
    clear
    echo -e "${CYAN}=== System Information ===${NC}"
    echo ""
    echo "System: $(lsb_release -d 2>/dev/null | cut -f2 || uname -s)"
    echo "Kernel: $(uname -r)"
    echo "Architecture: $(uname -m)"
    echo ""
    echo -e "${CYAN}=== Detected Software ===${NC}"
    echo "Wine: ${DETECTED_WINE:-Not found}"
    echo "Proton: ${DETECTED_PROTON:-Not found}"
    echo "Steam: ${DETECTED_STEAM:-Not found}"
    echo ""
    echo -e "${CYAN}=== BarrierLayer Status ===${NC}"
    echo "Hook Library: $([ -f "$HOOK_LIBRARY" ] && echo "✅ Available" || echo "❌ Missing")"
    echo "Configuration: $([ -f "$CONFIG_FILE" ] && echo "✅ Configured" || echo "❌ Not configured")"
    echo ""
}

# Ajuda
show_help() {
    echo -e "${CYAN}BarrierLayer Launcher v2.0${NC}"
    echo ""
    echo "Usage: $0 [OPTIONS] <executable> [args...]"
    echo ""
    echo -e "${YELLOW}Options:${NC}"
    echo "  -w, --wine          Force Wine mode"
    echo "  -p, --proton        Force Proton mode"
    echo "  -n, --native        Force native mode"
    echo "  -i, --interactive   Show interactive menu"
    echo "  -v, --verbose       Verbose output"
    echo "  -h, --help          Show this help"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  $0 game.exe                    # Auto-detect environment"
    echo "  $0 -w game.exe                 # Force Wine"
    echo "  $0 -p game.exe                 # Force Proton"
    echo "  $0 -i                          # Interactive mode"
    echo "  $0 /usr/bin/steam steam://run/123456  # Native Steam"
    echo ""
    echo -e "${YELLOW}Environment Variables:${NC}"
    echo "  BARRIERLAYER_LOG_LEVEL=0-4     # Log level (0=trace, 4=error)"
    echo "  BARRIERLAYER_DETAILED=1        # Detailed logging"
    echo "  BARRIERLAYER_STACK_TRACE=1     # Stack traces"
    echo ""
}

# Função principal
main() {
    # Carregar configuração padrão do config.mk
    if [[ -f "$CONFIG_FILE" ]]; then
        DEFAULT_MODE_FROM_CONFIG=$(grep -E '^\s*DEFAULT_LAUNCHER\s*=' "$CONFIG_FILE" | cut -d'=' -f2 | tr -d '[:space:]')
        if [[ -n "$DEFAULT_MODE_FROM_CONFIG" && "$DEFAULT_MODE_FROM_CONFIG" != "auto" ]]; then
            LAUNCH_MODE="$DEFAULT_MODE_FROM_CONFIG"
            log_debug "Default launch mode loaded from config.mk: $LAUNCH_MODE"
        fi
    fi

    # Parse argumentos (eles podem sobrepor a configuração padrão)
    while [[ $# -gt 0 ]]; do
        case $1 in
            -w|--wine)
                LAUNCH_MODE="wine"
                shift
                ;;
            -p|--proton)
                LAUNCH_MODE="proton"
                shift
                ;;
            -n|--native)
                LAUNCH_MODE="native"
                shift
                ;;
            -i|--interactive)
                show_banner
                detect_wine
                detect_proton
                detect_steam
                show_interactive_menu
                if [[ -z "$TARGET_EXECUTABLE" ]]; then
                    read -p "Enter executable path: " TARGET_EXECUTABLE
                fi
                break
                ;;
            -v|--verbose)
                VERBOSE=1
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            -*)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
            *)
                TARGET_EXECUTABLE="$1"
                shift
                break
                ;;
        esac
    done
    
    # Verificar se executável foi fornecido
    if [[ -z "$TARGET_EXECUTABLE" ]]; then
        show_help
        exit 1
    fi
    
    # Verificar se executável existe
    if [[ ! -f "$TARGET_EXECUTABLE" && ! -x "$TARGET_EXECUTABLE" ]]; then
        log_error "Executable not found: $TARGET_EXECUTABLE"
        exit 1
    fi
    
    # Detectar ambiente se não especificado
    if [[ -z "$LAUNCH_MODE" ]]; then
        detect_environment
    fi
    
    # Configurar BarrierLayer
    setup_barrierlayer
    
    # Lançar aplicação
    launch_application "$TARGET_EXECUTABLE" "$@"
}

# Executar se chamado diretamente
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi