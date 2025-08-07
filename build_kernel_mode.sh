#!/bin/bash

# Script para compilar e testar o BarrierLayer em modo kernel
# Autor: BarrierLayer Team
# Versão: 2.0

set -e

echo "🚀 BarrierLayer Kernel Mode Build System"
echo "========================================"

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

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

# Verificar se está rodando como root para módulo do kernel
check_root() {
    if [[ $EUID -ne 0 ]]; then
        log_warning "Algumas operações requerem privilégios de root"
        log_info "Para carregar o módulo do kernel, execute: sudo $0"
    fi
}

# Verificar dependências
check_dependencies() {
    log_step "Verificando dependências..."
    
    local deps=("gcc" "make" "nasm" "pkg-config")
    local missing=()
    
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            missing+=("$dep")
        fi
    done
    
    # Verificar headers do kernel
    if [[ ! -d "/lib/modules/$(uname -r)/build" ]]; then
        missing+=("linux-headers-$(uname -r)")
    fi
    
    # Verificar bibliotecas
    if ! pkg-config --exists gtk+-3.0; then
        missing+=("libgtk-3-dev")
    fi
    
    if [[ ${#missing[@]} -gt 0 ]]; then
        log_error "Dependências faltando: ${missing[*]}"
        log_info "Instalando dependências automaticamente..."
        
        if command -v apt-get &> /dev/null; then
            sudo apt-get update
            sudo apt-get install -y build-essential linux-headers-$(uname -r) nasm libgtk-3-dev libseccomp-dev
        elif command -v yum &> /dev/null; then
            sudo yum install -y gcc make nasm gtk3-devel libseccomp-devel kernel-devel
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm gcc make nasm gtk3 libseccomp linux-headers
        else
            log_error "Gerenciador de pacotes não suportado. Instale manualmente: ${missing[*]}"
            exit 1
        fi
    fi
    
    log_success "Todas as dependências estão disponíveis"
}

# Compilar userspace
build_userspace() {
    log_step "Compilando componentes userspace..."
    
    # Limpar builds anteriores
    make clean 2>/dev/null || true
    
    # Compilar com ultra logging
    export CFLAGS="-DULTRA_LOGGING_ENABLED -g -O2 -Wall -Wextra"
    
    if make build; then
        log_success "Componentes userspace compilados com sucesso"
        
        # Mostrar tamanhos dos binários
        log_info "Tamanhos dos binários:"
        ls -lh bin/ | grep -E '\.(so|exe)$|barrierlayer' || true
    else
        log_error "Falha na compilação userspace"
        return 1
    fi
}

# Compilar módulo do kernel
build_kernel_module() {
    log_step "Compilando módulo do kernel..."
    
    cd kernel/
    
    if make all; then
        log_success "Módulo do kernel compilado com sucesso"
        
        # Mostrar informações do módulo
        if [[ -f "barrierlayer_kernel.ko" ]]; then
            log_info "Informações do módulo:"
            modinfo barrierlayer_kernel.ko | head -10
        fi
    else
        log_error "Falha na compilação do módulo do kernel"
        cd ..
        return 1
    fi
    
    cd ..
}

# Criar diretórios necessários
setup_directories() {
    log_step "Criando diretórios necessários..."
    
    mkdir -p /tmp/wine_files
    mkdir -p /tmp/barrierlayer_logs
    chmod 777 /tmp/wine_files /tmp/barrierlayer_logs
    
    log_success "Diretórios criados"
}

# Instalar módulo do kernel (requer root)
install_kernel_module() {
    if [[ $EUID -ne 0 ]]; then
        log_warning "Pulando instalação do módulo (requer root)"
        return 0
    fi
    
    log_step "Instalando módulo do kernel..."
    
    cd kernel/
    
    # Remover módulo anterior se existir
    if lsmod | grep -q barrierlayer_kernel; then
        log_info "Removendo módulo anterior..."
        rmmod barrierlayer_kernel || true
    fi
    
    # Instalar novo módulo
    if make install; then
        log_success "Módulo do kernel instalado com sucesso"
        
        # Verificar se foi carregado
        if lsmod | grep -q barrierlayer_kernel; then
            log_success "Módulo carregado e ativo"
            
            # Mostrar logs iniciais
            log_info "Logs iniciais do kernel:"
            dmesg | grep BarrierLayer | tail -5
            
            # Verificar se /proc/barrierlayer existe
            if [[ -f /proc/barrierlayer ]]; then
                log_success "Interface /proc/barrierlayer disponível"
            else
                log_warning "Interface /proc/barrierlayer não encontrada"
            fi
        else
            log_error "Módulo não foi carregado corretamente"
        fi
    else
        log_error "Falha na instalação do módulo"
        cd ..
        return 1
    fi
    
    cd ..
}

# Executar testes
run_tests() {
    log_step "Executando testes..."
    
    # Teste básico de carregamento da biblioteca
    log_info "Testando carregamento da biblioteca..."
    if ./simple_test; then
        log_success "Biblioteca carrega corretamente"
    else
        log_warning "Problemas no carregamento da biblioteca"
    fi
    
    # Teste do ultra logger
    log_info "Testando ultra logger..."
    export BARRIERLAYER_LOG_LEVEL=0
    export BARRIERLAYER_DETAILED=1
    export BARRIERLAYER_STACK_TRACE=1
    
    # Executar um comando simples para gerar logs
    echo "Gerando logs de teste..."
    LD_PRELOAD=./bin/barrierlayer_hook.so ls /tmp >/dev/null 2>&1 || true
    
    # Verificar se logs foram gerados
    if ls /tmp/barrierlayer_ultra_*.log 2>/dev/null; then
        log_success "Ultra logger funcionando - logs gerados"
        log_info "Primeiras linhas do log:"
        head -20 /tmp/barrierlayer_ultra_*.log | head -20
    else
        log_warning "Ultra logger pode não estar funcionando"
    fi
    
    # Teste do módulo do kernel (se instalado)
    if [[ $EUID -eq 0 ]] && lsmod | grep -q barrierlayer_kernel; then
        log_info "Testando módulo do kernel..."
        
        # Executar comando para gerar atividade
        echo "Gerando atividade para o módulo do kernel..."
        ls /proc >/dev/null 2>&1
        
        # Verificar logs do kernel
        if [[ -f /proc/barrierlayer ]]; then
            log_info "Logs do módulo do kernel:"
            head -10 /proc/barrierlayer
        fi
    fi
}

# Mostrar status final
show_status() {
    log_step "Status final do sistema..."
    
    echo -e "${CYAN}=== BarrierLayer Status ===${NC}"
    
    # Status userspace
    echo -e "${YELLOW}Userspace:${NC}"
    if [[ -f "bin/barrierlayer_hook.so" ]]; then
        echo "  ✅ Biblioteca principal: $(ls -lh bin/barrierlayer_hook.so | awk '{print $5}')"
    else
        echo "  ❌ Biblioteca principal: Não encontrada"
    fi
    
    if [[ -f "bin/barrierlayer" ]]; then
        echo "  ✅ Executável principal: $(ls -lh bin/barrierlayer | awk '{print $5}')"
    else
        echo "  ❌ Executável principal: Não encontrado"
    fi
    
    if [[ -f "bin/barrierlayer_gui" ]]; then
        echo "  ✅ Interface gráfica: $(ls -lh bin/barrierlayer_gui | awk '{print $5}')"
    else
        echo "  ❌ Interface gráfica: Não encontrada"
    fi
    
    # Status kernel
    echo -e "${YELLOW}Kernel Mode:${NC}"
    if lsmod | grep -q barrierlayer_kernel; then
        echo "  ✅ Módulo do kernel: Carregado e ativo"
        if [[ -f /proc/barrierlayer ]]; then
            echo "  ✅ Interface /proc: Disponível"
        else
            echo "  ❌ Interface /proc: Não disponível"
        fi
    else
        echo "  ❌ Módulo do kernel: Não carregado"
    fi
    
    # Status logs
    echo -e "${YELLOW}Logging:${NC}"
    local log_count=$(ls /tmp/barrierlayer_ultra_*.log 2>/dev/null | wc -l)
    if [[ $log_count -gt 0 ]]; then
        echo "  ✅ Ultra logs: $log_count arquivo(s) ativo(s)"
        local total_size=$(du -sh /tmp/barrierlayer_ultra_*.log 2>/dev/null | awk '{sum+=$1} END {print sum}' || echo "0")
        echo "  📊 Tamanho total: ${total_size}B"
    else
        echo "  ❌ Ultra logs: Nenhum arquivo encontrado"
    fi
    
    echo ""
    echo -e "${GREEN}🎯 BarrierLayer está pronto para uso!${NC}"
    echo ""
    echo -e "${CYAN}Comandos úteis:${NC}"
    echo "  • Ver logs kernel: sudo cat /proc/barrierlayer"
    echo "  • Ver logs ultra: cat /tmp/barrierlayer_ultra_*.log"
    echo "  • Monitor kernel: cd kernel && make monitor"
    echo "  • Usar com jogo: LD_PRELOAD=./bin/barrierlayer_hook.so <comando>"
    echo "  • Interface gráfica: ./bin/barrierlayer_gui"
}

# Função principal
main() {
    echo "Iniciando build do BarrierLayer em modo kernel..."
    echo ""
    
    check_root
    check_dependencies
    setup_directories
    
    # Build userspace
    if ! build_userspace; then
        log_error "Falha no build userspace"
        exit 1
    fi
    
    # Build kernel module
    if ! build_kernel_module; then
        log_error "Falha no build do módulo kernel"
        exit 1
    fi
    
    # Install kernel module (se root)
    install_kernel_module
    
    # Run tests
    run_tests
    
    # Show final status
    show_status
    
    log_success "Build completo! BarrierLayer está pronto para uso em modo kernel."
}

# Verificar argumentos
case "${1:-}" in
    "clean")
        log_info "Limpando builds..."
        make clean 2>/dev/null || true
        cd kernel && make clean 2>/dev/null || true
        rm -f /tmp/barrierlayer_ultra_*.log
        log_success "Limpeza concluída"
        ;;
    "kernel-only")
        check_dependencies
        build_kernel_module
        install_kernel_module
        ;;
    "userspace-only")
        check_dependencies
        setup_directories
        build_userspace
        run_tests
        ;;
    "help"|"-h"|"--help")
        echo "Uso: $0 [opção]"
        echo ""
        echo "Opções:"
        echo "  (nenhuma)     - Build completo (userspace + kernel)"
        echo "  clean         - Limpar todos os builds"
        echo "  kernel-only   - Compilar apenas módulo do kernel"
        echo "  userspace-only- Compilar apenas componentes userspace"
        echo "  help          - Mostrar esta ajuda"
        ;;
    *)
        main
        ;;
esac