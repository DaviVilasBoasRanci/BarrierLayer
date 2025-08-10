#!/bin/bash

# --- Configuration ---
INSTALL_PREFIX="/usr/local"
BIN_DIR="$INSTALL_PREFIX/bin"
LIB_DIR="$INSTALL_PREFIX/lib"
SHARE_DIR="$INSTALL_PREFIX/share/barrierlayer"
KERNEL_MODULE_NAME="barrierlayer_minimal" # From kernel/barrierlayer_minimal.ko

# --- Colors for output ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# --- Helper Functions ---
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

check_sudo() {
    if [[ "$EUID" -ne 0 ]]; then
        log_error "This script requires root privileges. Please run with sudo."
        exit 1
    fi
}

# --- Main Installation Logic ---
main() {
    check_sudo

    log_info "Starting BarrierLayer automated installation..."

    # 1. Build BarrierLayer
    log_info "Building BarrierLayer components..."
    if ! make; then
        log_error "BarrierLayer build failed. Please check the output above."
        exit 1
    fi
    log_success "BarrierLayer components built successfully."

    # 2. Install Userspace Components (using make install)
    log_info "Installing userspace components to $INSTALL_PREFIX..."
    if ! make install; then
        log_error "Userspace installation failed. Please check the output above."
        exit 1
    fi
    log_success "Userspace components installed."

    # 3. Kernel Module Installation and Loading
    log_info "Checking for kernel module installation..."
    # Read BUILD_KERNEL from config.mk
    BUILD_KERNEL=$(grep "^BUILD_KERNEL =" config.mk | cut -d'=' -f2 | sed 's/^[[:space:]]*//')
    if [[ "$BUILD_KERNEL" == "Y" ]]; then
        log_info "Kernel module build is enabled. Installing and loading kernel module..."
        KERNEL_MODULE_PATH="/lib/modules/$(uname -r)/extra/$KERNEL_MODULE_NAME.ko"
        
        # Copy module
        if ! cp kernel/$KERNEL_MODULE_NAME.ko "$KERNEL_MODULE_PATH"; then
            log_error "Failed to copy kernel module. Aborting kernel module installation."
        else
            log_success "Kernel module copied to $KERNEL_MODULE_PATH."
            # Update module dependencies
            log_info "Updating kernel module dependencies..."
            if ! depmod -a; then
                log_warn "Failed to update kernel module dependencies. Module might not load correctly."
            else
                log_success "Kernel module dependencies updated."
            fi

            # Load module
            log_info "Loading kernel module..."
            if ! modprobe "$KERNEL_MODULE_NAME"; then
                log_error "Failed to load kernel module. You might need to load it manually: 'sudo modprobe $KERNEL_MODULE_NAME'."
            else
                log_success "Kernel module loaded successfully."
                # Add to modules-load.d for automatic loading on boot
                log_info "Configuring kernel module to load on boot..."
                echo "$KERNEL_MODULE_NAME" | sudo tee "/etc/modules-load.d/$KERNEL_MODULE_NAME.conf" > /dev/null
                log_success "Kernel module configured for automatic loading on boot."
            fi
        fi
    else
        log_info "Kernel module build is disabled. Skipping kernel module installation."
    fi

    # 4. Create Aliases
    log_info "Creating shell aliases..."
    SHELL_RC_FILE=""
    if [[ -f "$HOME/.bashrc" ]]; then
        SHELL_RC_FILE="$HOME/.bashrc"
    elif [[ -f "$HOME/.zshrc" ]]; then
        SHELL_RC_FILE="$HOME/.zshrc"
    else
        log_warn "Could not find .bashrc or .zshrc. Please add aliases manually."
    fi

    if [[ -n "$SHELL_RC_FILE" ]]; then
        {
            echo ""
            echo "# BarrierLayer Aliases"
            echo "alias barrierlayer=\"$BIN_DIR/barrierlayer\""
            echo "alias barrierlayer-gui=\"$BIN_DIR/barrierlayer_gui\""
            echo "alias barrierlayer-config=\"make -C $(pwd) configs\" # Use absolute path for make configs"
            echo "alias barrierlayer-uninstall=\"sudo make -C $(pwd) uninstall\" # Use absolute path for make uninstall"
        } >> "$SHELL_RC_FILE"
        log_success "Aliases added to $SHELL_RC_FILE. Please run 'source $SHELL_RC_FILE' or restart your terminal."
    fi

    log_success "BarrierLayer installation complete!"
    log_info "You can now use 'barrierlayer', 'barrierlayer-gui', 'barrierlayer-config' (after sourcing your shell RC file)."
}

# --- Run Main Function ---
main "$@"
