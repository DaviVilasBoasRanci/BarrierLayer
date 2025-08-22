#!/bin/bash

# BarrierLayer Integrity Checker using AIDE
#
# Usage:
#   sudo ./integrity_check.sh init <directory_to_monitor>
#   sudo ./integrity_check.sh check <directory_to_monitor>

set -e

# Cores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

if [[ "$EUID" -ne 0 ]]; then
  log_error "This script must be run as root (sudo) to manage AIDE database."
  exit 1
fi

if ! command -v aide &> /dev/null; then
    log_error "AIDE is not installed. Please install it first."
    log_info "On Debian/Ubuntu: sudo apt-get install aide"
    log_info "On Fedora: sudo dnf install aide"
    exit 1
fi

ACTION="$1"
TARGET_DIR="$2"

if [[ -z "$ACTION" || -z "$TARGET_DIR" ]]; then
    echo "Usage: sudo $0 <init|check> <directory_to_monitor>"
    exit 1
fi

TARGET_DIR_ABS="$(realpath "$TARGET_DIR")"
DB_DIR="/var/lib/barrierlayer/aide"
DB_PATH="$DB_DIR/$(basename "$TARGET_DIR_ABS").db.gz"
AIDE_CONF="/tmp/aide.conf.$$"

# Gerar uma configuração AIDE mínima
cat > "$AIDE_CONF" << EOM
# AIDE configuration for BarrierLayer
database=file:$DB_PATH
database_out=file:$DB_PATH

# Regras de verificação (muito detalhadas)
FIPSR = p+i+n+u+g+s+m+c+md5+sha1+sha256+sha512+ripemd160+tiger

# Diretório a ser monitorado
$TARGET_DIR_ABS FIPSR
EOM

# Garantir que o diretório do banco de dados exista
mkdir -p "$DB_DIR"

case "$ACTION" in
    init)
        log_info "Initializing AIDE database for: $TARGET_DIR_ABS"
        log_info "This will create a snapshot of the current state."
        
        if [[ -f "$DB_PATH" ]]; then
            read -p "Database already exists. Overwrite? (y/N): " choice
            if [[ "$choice" != "y" && "$choice" != "Y" ]]; then
                log_info "Initialization aborted."
                rm -f "$AIDE_CONF"
                exit 0
            fi
        fi
        
        aide --config="$AIDE_CONF" --init
        log_success "AIDE database initialized at $DB_PATH"
        ;;
    check)
        log_info "Running integrity check for: $TARGET_DIR_ABS"
        
        if [[ ! -f "$DB_PATH" ]]; then
            log_error "AIDE database not found. Please run 'init' first."
            rm -f "$AIDE_CONF"
            exit 1
        fi
        
        # O AIDE retorna um código de saída diferente de 0 se encontrar mudanças
        if aide --config="$AIDE_CONF" --check; then
            log_success "Integrity check passed. No changes detected."
        else
            log_error "Integrity check FAILED. Changes detected!"
            log_info "Review the output above for details."
            rm -f "$AIDE_CONF"
            exit 1 # Falhar para que o launcher possa pegar o erro
        fi
        ;;
    *)
        log_error "Invalid action: $ACTION. Use 'init' or 'check'."
        rm -f "$AIDE_CONF"
        exit 1
        ;;
esac

# Limpeza
rm -f "$AIDE_CONF"
