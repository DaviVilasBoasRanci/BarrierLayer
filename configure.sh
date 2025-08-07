#!/bin/bash

# BarrierLayer Configuration System
# Autor: BarrierLayer Team
# Versão: 2.0

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Arquivo de configuração
CONFIG_FILE="config.mk"
CONFIG_H="src/include/config.h"

# Função para log colorido
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

log_step() {
    echo -e "${PURPLE}[STEP]${NC} $1"
}

# Banner
show_banner() {
    echo -e "${CYAN}"
    cat << "EOF"
╔══════════════════════════════════════════════════════════════╗
║                    BarrierLayer v2.0                        ║
║              Configuration & Build System                   ║
║                                                              ║
║  Advanced Anti-Cheat Bypass for Linux Gaming                ║
║  Supports: Wine, Proton, EAC, BattlEye                      ║
╚══════════════════════════════════════════════════════════════╝
EOF
    echo -e "${NC}"
}

# Detectar sistema
detect_system() {
    log_step "Detectando sistema..."
    
    # Detectar distribuição
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        DISTRO_VERSION=$VERSION_ID
    else
        DISTRO="unknown"
        DISTRO_VERSION="unknown"
    fi
    
    # Detectar arquitetura
    ARCH=$(uname -m)
    KERNEL_VERSION=$(uname -r)
    
    log_info "Sistema: $PRETTY_NAME ($ARCH)"
    log_info "Kernel: $KERNEL_VERSION"
    
    # Verificar se é compatível
    if [[ "$ARCH" != "x86_64" ]]; then
        log_error "BarrierLayer requer arquitetura x86_64"
        exit 1
    fi
}

# Detectar Wine/Proton
detect_wine_proton() {
    log_step "Detectando Wine e Proton..."
    
    # Detectar Wine
    WINE_PATH=""
    WINE_VERSION=""
    if command -v wine &> /dev/null; then
        WINE_PATH=$(which wine)
        WINE_VERSION=$(wine --version 2>/dev/null | head -1)
        log_success "Wine encontrado: $WINE_VERSION em $WINE_PATH"
    else
        log_warning "Wine não encontrado"
    fi
    
    # Detectar Proton
    PROTON_PATHS=(
        "$HOME/.steam/steam/steamapps/common/Proton*"
        "$HOME/.local/share/Steam/steamapps/common/Proton*"
        "/usr/share/steam/steamapps/common/Proton*"
    )
    
    PROTON_INSTALLATIONS=()
    for pattern in "${PROTON_PATHS[@]}"; do
        for path in $pattern; do
            if [[ -d "$path" && -f "$path/proton" ]]; then
                version=$(basename "$path")
                PROTON_INSTALLATIONS+=("$path:$version")
                log_success "Proton encontrado: $version em $path"
            fi
        done
    done
    
    if [[ ${#PROTON_INSTALLATIONS[@]} -eq 0 ]]; then
        log_warning "Nenhuma instalação do Proton encontrada"
    fi
    
    # Detectar Steam
    STEAM_PATH=""
    if command -v steam &> /dev/null; then
        STEAM_PATH=$(which steam)
        log_success "Steam encontrado em $STEAM_PATH"
    else
        log_warning "Steam não encontrado"
    fi
}

# Menu principal de configuração
show_config_menu() {
    while true; do
        clear
        show_banner
        
        echo -e "${YELLOW}=== Menu de Configuração ===${NC}"
        echo ""
        echo "1) 🍷 Configurar Wine"
        echo "2) 🚂 Configurar Proton"
        echo "3) 🛡️  Configurar Anti-Cheat"
        echo "4) 🔧 Configurar Build"
        echo "5) 📊 Configurar Logging"
        echo "6) 🎯 Configurar Stealth"
        echo "7) 🖥️  Configurar Interface"
        echo "8) 📋 Ver Configuração Atual"
        echo "9) 💾 Salvar e Sair"
        echo "0) ❌ Sair sem Salvar"
        echo ""
        read -p "Escolha uma opção [0-9]: " choice
        
        case $choice in
            1) configure_wine ;;
            2) configure_proton ;;
            3) configure_anticheat ;;
            4) configure_build ;;
            5) configure_logging ;;
            6) configure_stealth ;;
            7) configure_interface ;;
            8) show_current_config ;;
            9) save_config && exit 0 ;;
            0) exit 0 ;;
            *) log_error "Opção inválida!" && sleep 1 ;;
        esac
    done
}

# Configurar Wine
configure_wine() {
    clear
    echo -e "${YELLOW}=== Configuração do Wine ===${NC}"
    echo ""
    
    # Usar Wine?
    read -p "Habilitar suporte ao Wine? [Y/n]: " enable_wine
    enable_wine=${enable_wine:-Y}
    
    if [[ "$enable_wine" =~ ^[Yy]$ ]]; then
        USE_WINE=1
        
        # Caminho do Wine
        if [[ -n "$WINE_PATH" ]]; then
            echo "Wine detectado: $WINE_PATH"
            read -p "Usar este caminho? [Y/n]: " use_detected
            use_detected=${use_detected:-Y}
            
            if [[ "$use_detected" =~ ^[Yy]$ ]]; then
                WINE_BINARY="$WINE_PATH"
            else
                read -p "Digite o caminho para o wine: " WINE_BINARY
            fi
        else
            read -p "Digite o caminho para o wine: " WINE_BINARY
        fi
        
        # Prefixo do Wine
        read -p "Caminho do prefixo Wine [$HOME/.wine]: " wine_prefix
        WINE_PREFIX=${wine_prefix:-$HOME/.wine}
        
        # Arquitetura
        echo ""
        echo "Arquitetura do Wine:"
        echo "1) win64 (64-bit)"
        echo "2) win32 (32-bit)"
        read -p "Escolha [1-2]: " wine_arch_choice
        
        case $wine_arch_choice in
            1) WINE_ARCH="win64" ;;
            2) WINE_ARCH="win32" ;;
            *) WINE_ARCH="win64" ;;
        esac
        
        log_success "Wine configurado: $WINE_BINARY ($WINE_ARCH)"
    else
        USE_WINE=0
        log_info "Suporte ao Wine desabilitado"
    fi
    
    read -p "Pressione Enter para continuar..."
}

# Configurar Proton
configure_proton() {
    clear
    echo -e "${YELLOW}=== Configuração do Proton ===${NC}"
    echo ""
    
    # Usar Proton?
    read -p "Habilitar suporte ao Proton? [Y/n]: " enable_proton
    enable_proton=${enable_proton:-Y}
    
    if [[ "$enable_proton" =~ ^[Yy]$ ]]; then
        USE_PROTON=1
        
        if [[ ${#PROTON_INSTALLATIONS[@]} -gt 0 ]]; then
            echo "Instalações do Proton encontradas:"
            for i in "${!PROTON_INSTALLATIONS[@]}"; do
                IFS=':' read -r path version <<< "${PROTON_INSTALLATIONS[$i]}"
                echo "$((i+1))) $version"
            done
            echo "$((${#PROTON_INSTALLATIONS[@]}+1))) Caminho personalizado"
            
            read -p "Escolha uma instalação [1-$((${#PROTON_INSTALLATIONS[@]}+1))]: " proton_choice
            
            if [[ "$proton_choice" -le "${#PROTON_INSTALLATIONS[@]}" ]]; then
                IFS=':' read -r PROTON_PATH PROTON_VERSION <<< "${PROTON_INSTALLATIONS[$((proton_choice-1))]}"
            else
                read -p "Digite o caminho para o Proton: " PROTON_PATH
                PROTON_VERSION=$(basename "$PROTON_PATH")
            fi
        else
            read -p "Digite o caminho para o Proton: " PROTON_PATH
            PROTON_VERSION=$(basename "$PROTON_PATH")
        fi
        
        # Configurações do Proton
        read -p "Usar DXVK? [Y/n]: " use_dxvk
        USE_DXVK=${use_dxvk:-Y}
        
        read -p "Usar VKD3D? [Y/n]: " use_vkd3d
        USE_VKD3D=${use_vkd3d:-Y}
        
        read -p "Usar Esync? [Y/n]: " use_esync
        USE_ESYNC=${use_esync:-Y}
        
        read -p "Usar Fsync? [Y/n]: " use_fsync
        USE_FSYNC=${use_fsync:-Y}
        
        log_success "Proton configurado: $PROTON_VERSION"
    else
        USE_PROTON=0
        log_info "Suporte ao Proton desabilitado"
    fi
    
    read -p "Pressione Enter para continuar..."
}

# Configurar Anti-Cheat
configure_anticheat() {
    clear
    echo -e "${YELLOW}=== Configuração Anti-Cheat ===${NC}"
    echo ""
    
    echo "Sistemas Anti-Cheat suportados:"
    echo ""
    
    # Easy Anti-Cheat
    read -p "Habilitar proteção contra EAC (Easy Anti-Cheat)? [Y/n]: " enable_eac
    ENABLE_EAC=${enable_eac:-Y}
    
    # BattlEye
    read -p "Habilitar proteção contra BattlEye? [Y/n]: " enable_battleye
    ENABLE_BATTLEYE=${enable_battleye:-Y}
    
    # Vanguard
    read -p "Habilitar proteção contra Vanguard? [Y/n]: " enable_vanguard
    ENABLE_VANGUARD=${enable_vanguard:-Y}
    
    # Proteções gerais
    echo ""
    echo "Proteções gerais:"
    read -p "Habilitar detecção de debugger? [Y/n]: " enable_debugger_detection
    ENABLE_DEBUGGER_DETECTION=${enable_debugger_detection:-Y}
    
    read -p "Habilitar proteção de memória? [Y/n]: " enable_memory_protection
    ENABLE_MEMORY_PROTECTION=${enable_memory_protection:-Y}
    
    read -p "Habilitar mascaramento de processo? [Y/n]: " enable_process_masking
    ENABLE_PROCESS_MASKING=${enable_process_masking:-Y}
    
    log_success "Configurações anti-cheat definidas"
    read -p "Pressione Enter para continuar..."
}

# Configurar Build
configure_build() {
    clear
    echo -e "${YELLOW}=== Configuração de Build ===${NC}"
    echo ""
    
    # Modo de build
    echo "Modo de build:"
    echo "1) Debug (com símbolos de debug)"
    echo "2) Release (otimizado)"
    echo "3) RelWithDebInfo (otimizado com debug)"
    read -p "Escolha [1-3]: " build_mode
    
    case $build_mode in
        1) BUILD_MODE="debug" ;;
        2) BUILD_MODE="release" ;;
        3) BUILD_MODE="relwithdebinfo" ;;
        *) BUILD_MODE="release" ;;
    esac
    
    # Compilar kernel module?
    read -p "Compilar módulo do kernel? [Y/n]: " build_kernel
    BUILD_KERNEL=${build_kernel:-Y}
    
    # Compilar GUI?
    read -p "Compilar interface gráfica? [Y/n]: " build_gui
    BUILD_GUI=${build_gui:-Y}
    
    # Otimizações
    echo ""
    echo "Otimizações:"
    read -p "Habilitar otimizações LTO? [y/N]: " enable_lto
    ENABLE_LTO=${enable_lto:-N}
    
    read -p "Habilitar otimizações nativas? [y/N]: " enable_native
    ENABLE_NATIVE=${enable_native:-N}
    
    log_success "Configurações de build definidas: $BUILD_MODE"
    read -p "Pressione Enter para continuar..."
}

# Configurar Logging
configure_logging() {
    clear
    echo -e "${YELLOW}=== Configuração de Logging ===${NC}"
    echo ""
    
    # Nível de log
    echo "Nível de logging:"
    echo "0) TRACE (muito detalhado)"
    echo "1) DEBUG (detalhado)"
    echo "2) INFO (informativo)"
    echo "3) WARN (apenas avisos)"
    echo "4) ERROR (apenas erros)"
    read -p "Escolha [0-4]: " log_level
    LOG_LEVEL=${log_level:-2}
    
    # Ultra logging
    read -p "Habilitar ultra logging? [Y/n]: " enable_ultra_logging
    ENABLE_ULTRA_LOGGING=${enable_ultra_logging:-Y}
    
    # Stack traces
    read -p "Habilitar stack traces? [Y/n]: " enable_stack_traces
    ENABLE_STACK_TRACES=${enable_stack_traces:-Y}
    
    # Log para arquivo
    read -p "Salvar logs em arquivo? [Y/n]: " enable_file_logging
    ENABLE_FILE_LOGGING=${enable_file_logging:-Y}
    
    if [[ "$ENABLE_FILE_LOGGING" =~ ^[Yy]$ ]]; then
        read -p "Diretório de logs [/tmp]: " log_directory
        LOG_DIRECTORY=${log_directory:-/tmp}
        
        read -p "Tamanho máximo do log (MB) [100]: " max_log_size
        MAX_LOG_SIZE=${max_log_size:-100}
    fi
    
    log_success "Configurações de logging definidas"
    read -p "Pressione Enter para continuar..."
}

# Configurar Stealth
configure_stealth() {
    clear
    echo -e "${YELLOW}=== Configuração de Stealth ===${NC}"
    echo ""
    
    echo "Recursos de stealth:"
    
    # Remover LD_PRELOAD
    read -p "Remover LD_PRELOAD do ambiente? [Y/n]: " remove_ld_preload
    REMOVE_LD_PRELOAD=${remove_ld_preload:-Y}
    
    # Unlinking
    read -p "Habilitar library unlinking? [Y/n]: " enable_unlinking
    ENABLE_UNLINKING=${enable_unlinking:-Y}
    
    # Mascaramento de arquivos
    read -p "Bloquear acesso a arquivos sensíveis? [Y/n]: " block_sensitive_files
    BLOCK_SENSITIVE_FILES=${block_sensitive_files:-Y}
    
    # Randomização
    read -p "Randomizar nomes de processos? [y/N]: " randomize_process_names
    RANDOMIZE_PROCESS_NAMES=${randomize_process_names:-N}
    
    # Injeção de código
    read -p "Habilitar injeção de código? [y/N]: " enable_code_injection
    ENABLE_CODE_INJECTION=${enable_code_injection:-N}
    
    log_success "Configurações de stealth definidas"
    read -p "Pressione Enter para continuar..."
}

# Configurar Interface
configure_interface() {
    clear
    echo -e "${YELLOW}=== Configuração de Interface ===${NC}"
    echo ""
    
    # Interface gráfica
    read -p "Habilitar interface gráfica? [Y/n]: " enable_gui
    ENABLE_GUI=${enable_gui:-Y}
    
    if [[ "$ENABLE_GUI" =~ ^[Yy]$ ]]; then
        echo "Toolkit de interface:"
        echo "1) GTK3"
        echo "2) GTK4"
        echo "3) Qt5"
        echo "4) Qt6"
        read -p "Escolha [1-4]: " gui_toolkit
        
        case $gui_toolkit in
            1) GUI_TOOLKIT="gtk3" ;;
            2) GUI_TOOLKIT="gtk4" ;;
            3) GUI_TOOLKIT="qt5" ;;
            4) GUI_TOOLKIT="qt6" ;;
            *) GUI_TOOLKIT="gtk3" ;;
        esac
    fi
    
    # Interface de linha de comando
    read -p "Habilitar interface de linha de comando avançada? [Y/n]: " enable_cli
    ENABLE_CLI=${enable_cli:-Y}
    
    # Tray icon
    read -p "Habilitar ícone na bandeja do sistema? [y/N]: " enable_tray
    ENABLE_TRAY=${enable_tray:-N}
    
    log_success "Configurações de interface definidas"
    read -p "Pressione Enter para continuar..."
}

# Mostrar configuração atual
show_current_config() {
    clear
    echo -e "${YELLOW}=== Configuração Atual ===${NC}"
    echo ""
    
    echo -e "${CYAN}Sistema:${NC}"
    echo "  Distribuição: $DISTRO $DISTRO_VERSION"
    echo "  Arquitetura: $ARCH"
    echo "  Kernel: $KERNEL_VERSION"
    echo ""
    
    echo -e "${CYAN}Wine/Proton:${NC}"
    echo "  Wine: ${USE_WINE:-0} (${WINE_BINARY:-N/A})"
    echo "  Proton: ${USE_PROTON:-0} (${PROTON_VERSION:-N/A})"
    echo ""
    
    echo -e "${CYAN}Anti-Cheat:${NC}"
    echo "  EAC: ${ENABLE_EAC:-Y}"
    echo "  BattlEye: ${ENABLE_BATTLEYE:-Y}"
    echo "  Vanguard: ${ENABLE_VANGUARD:-Y}"
    echo ""
    
    echo -e "${CYAN}Build:${NC}"
    echo "  Modo: ${BUILD_MODE:-release}"
    echo "  Kernel: ${BUILD_KERNEL:-Y}"
    echo "  GUI: ${BUILD_GUI:-Y}"
    echo ""
    
    echo -e "${CYAN}Logging:${NC}"
    echo "  Nível: ${LOG_LEVEL:-2}"
    echo "  Ultra Logging: ${ENABLE_ULTRA_LOGGING:-Y}"
    echo "  Stack Traces: ${ENABLE_STACK_TRACES:-Y}"
    echo ""
    
    echo -e "${CYAN}Stealth:${NC}"
    echo "  Remove LD_PRELOAD: ${REMOVE_LD_PRELOAD:-Y}"
    echo "  Library Unlinking: ${ENABLE_UNLINKING:-Y}"
    echo "  Block Sensitive Files: ${BLOCK_SENSITIVE_FILES:-Y}"
    echo ""
    
    read -p "Pressione Enter para continuar..."
}

# Salvar configuração
save_config() {
    log_step "Salvando configuração..."
    
    # Criar config.mk
    cat > "$CONFIG_FILE" << EOF
# BarrierLayer Configuration
# Generated by configure.sh on $(date)

# System Information
DISTRO = $DISTRO
DISTRO_VERSION = $DISTRO_VERSION
ARCH = $ARCH
KERNEL_VERSION = $KERNEL_VERSION

# Wine Configuration
USE_WINE = ${USE_WINE:-0}
WINE_BINARY = ${WINE_BINARY:-}
WINE_PREFIX = ${WINE_PREFIX:-}
WINE_ARCH = ${WINE_ARCH:-win64}

# Proton Configuration
USE_PROTON = ${USE_PROTON:-0}
PROTON_PATH = ${PROTON_PATH:-}
PROTON_VERSION = ${PROTON_VERSION:-}
USE_DXVK = ${USE_DXVK:-Y}
USE_VKD3D = ${USE_VKD3D:-Y}
USE_ESYNC = ${USE_ESYNC:-Y}
USE_FSYNC = ${USE_FSYNC:-Y}

# Anti-Cheat Configuration
ENABLE_EAC = ${ENABLE_EAC:-Y}
ENABLE_BATTLEYE = ${ENABLE_BATTLEYE:-Y}
ENABLE_VANGUARD = ${ENABLE_VANGUARD:-Y}
ENABLE_DEBUGGER_DETECTION = ${ENABLE_DEBUGGER_DETECTION:-Y}
ENABLE_MEMORY_PROTECTION = ${ENABLE_MEMORY_PROTECTION:-Y}
ENABLE_PROCESS_MASKING = ${ENABLE_PROCESS_MASKING:-Y}

# Build Configuration
BUILD_MODE = ${BUILD_MODE:-release}
BUILD_KERNEL = ${BUILD_KERNEL:-Y}
BUILD_GUI = ${BUILD_GUI:-Y}
ENABLE_LTO = ${ENABLE_LTO:-N}
ENABLE_NATIVE = ${ENABLE_NATIVE:-N}

# Logging Configuration
LOG_LEVEL = ${LOG_LEVEL:-2}
ENABLE_ULTRA_LOGGING = ${ENABLE_ULTRA_LOGGING:-Y}
ENABLE_STACK_TRACES = ${ENABLE_STACK_TRACES:-Y}
ENABLE_FILE_LOGGING = ${ENABLE_FILE_LOGGING:-Y}
LOG_DIRECTORY = ${LOG_DIRECTORY:-/tmp}
MAX_LOG_SIZE = ${MAX_LOG_SIZE:-100}

# Stealth Configuration
REMOVE_LD_PRELOAD = ${REMOVE_LD_PRELOAD:-Y}
ENABLE_UNLINKING = ${ENABLE_UNLINKING:-Y}
BLOCK_SENSITIVE_FILES = ${BLOCK_SENSITIVE_FILES:-Y}
RANDOMIZE_PROCESS_NAMES = ${RANDOMIZE_PROCESS_NAMES:-N}
ENABLE_CODE_INJECTION = ${ENABLE_CODE_INJECTION:-N}

# Interface Configuration
ENABLE_GUI = ${ENABLE_GUI:-Y}
GUI_TOOLKIT = ${GUI_TOOLKIT:-gtk3}
ENABLE_CLI = ${ENABLE_CLI:-Y}
ENABLE_TRAY = ${ENABLE_TRAY:-N}
EOF

    # Criar config.h
    mkdir -p "$(dirname "$CONFIG_H")"
    cat > "$CONFIG_H" << EOF
/* BarrierLayer Configuration Header */
/* Generated by configure.sh on $(date) */

#ifndef BARRIERLAYER_CONFIG_H
#define BARRIERLAYER_CONFIG_H

/* System Information */
#define BARRIERLAYER_DISTRO "$DISTRO"
#define BARRIERLAYER_DISTRO_VERSION "$DISTRO_VERSION"
#define BARRIERLAYER_ARCH "$ARCH"
#define BARRIERLAYER_KERNEL_VERSION "$KERNEL_VERSION"

/* Wine Configuration */
#define USE_WINE ${USE_WINE:-0}
#define WINE_BINARY "${WINE_BINARY:-}"
#define WINE_PREFIX "${WINE_PREFIX:-}"
#define WINE_ARCH "${WINE_ARCH:-win64}"

/* Proton Configuration */
#define USE_PROTON ${USE_PROTON:-0}
#define PROTON_PATH "${PROTON_PATH:-}"
#define PROTON_VERSION "${PROTON_VERSION:-}"
#define USE_DXVK $([ "${USE_DXVK:-Y}" = "Y" ] && echo 1 || echo 0)
#define USE_VKD3D $([ "${USE_VKD3D:-Y}" = "Y" ] && echo 1 || echo 0)
#define USE_ESYNC $([ "${USE_ESYNC:-Y}" = "Y" ] && echo 1 || echo 0)
#define USE_FSYNC $([ "${USE_FSYNC:-Y}" = "Y" ] && echo 1 || echo 0)

/* Anti-Cheat Configuration */
#define ENABLE_EAC $([ "${ENABLE_EAC:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_BATTLEYE $([ "${ENABLE_BATTLEYE:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_VANGUARD $([ "${ENABLE_VANGUARD:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_DEBUGGER_DETECTION $([ "${ENABLE_DEBUGGER_DETECTION:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_MEMORY_PROTECTION $([ "${ENABLE_MEMORY_PROTECTION:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_PROCESS_MASKING $([ "${ENABLE_PROCESS_MASKING:-Y}" = "Y" ] && echo 1 || echo 0)

/* Build Configuration */
#define BUILD_MODE "${BUILD_MODE:-release}"
#define BUILD_KERNEL $([ "${BUILD_KERNEL:-Y}" = "Y" ] && echo 1 || echo 0)
#define BUILD_GUI $([ "${BUILD_GUI:-Y}" = "Y" ] && echo 1 || echo 0)

/* Logging Configuration */
#define LOG_LEVEL ${LOG_LEVEL:-2}
#define ENABLE_ULTRA_LOGGING $([ "${ENABLE_ULTRA_LOGGING:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_STACK_TRACES $([ "${ENABLE_STACK_TRACES:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_FILE_LOGGING $([ "${ENABLE_FILE_LOGGING:-Y}" = "Y" ] && echo 1 || echo 0)
#define LOG_DIRECTORY "${LOG_DIRECTORY:-/tmp}"
#define MAX_LOG_SIZE ${MAX_LOG_SIZE:-100}

/* Stealth Configuration */
#define REMOVE_LD_PRELOAD $([ "${REMOVE_LD_PRELOAD:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_UNLINKING $([ "${ENABLE_UNLINKING:-Y}" = "Y" ] && echo 1 || echo 0)
#define BLOCK_SENSITIVE_FILES $([ "${BLOCK_SENSITIVE_FILES:-Y}" = "Y" ] && echo 1 || echo 0)
#define RANDOMIZE_PROCESS_NAMES $([ "${RANDOMIZE_PROCESS_NAMES:-N}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_CODE_INJECTION $([ "${ENABLE_CODE_INJECTION:-N}" = "Y" ] && echo 1 || echo 0)

/* Interface Configuration */
#define ENABLE_GUI $([ "${ENABLE_GUI:-Y}" = "Y" ] && echo 1 || echo 0)
#define GUI_TOOLKIT "${GUI_TOOLKIT:-gtk3}"
#define ENABLE_CLI $([ "${ENABLE_CLI:-Y}" = "Y" ] && echo 1 || echo 0)
#define ENABLE_TRAY $([ "${ENABLE_TRAY:-N}" = "Y" ] && echo 1 || echo 0)

#endif /* BARRIERLAYER_CONFIG_H */
EOF

    log_success "Configuração salva em $CONFIG_FILE e $CONFIG_H"
}

# Função principal
main() {
    show_banner
    detect_system
    detect_wine_proton
    
    if [[ "$1" == "--auto" ]]; then
        # Configuração automática com valores padrão
        log_info "Usando configuração automática..."
        save_config
    else
        show_config_menu
    fi
}

# Executar se chamado diretamente
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi