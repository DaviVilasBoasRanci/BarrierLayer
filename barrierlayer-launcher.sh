














#!/bin/bash

# BarrierLayer Smart Launcher
# Version: 7.0 (Final Diagnostics)

log_info() { echo -e "\033[0;34m[INFO]\033[0m $1"; }
log_error() { echo -e "\033[0;31m[ERROR]\033[0m $1"; }

main() {
    log_info "--- FINAL DIAGNOSTIC TEST ---"
    log_info "This test will determine if bwrap can run a simple command."

    local internal_cmd=("/usr/bin/ls" "-l" "/")
    
    local bwrap_args=(
        "--proc" "/proc"
        "--dev" "/dev"
        "--ro-bind" "/usr" "/usr"
        "--ro-bind" "/bin" "/bin" # Bind /bin just in case
        "--ro-bind" "/lib" "/lib"
        "--ro-bind" "/lib64" "/lib64"
    )

    log_info "Executing: bwrap ... -- /usr/bin/ls -l /"
    
    bwrap "${bwrap_args[@]}" -- "${internal_cmd[@]}"
    local exit_code=$?
    
    if [[ $exit_code -ne 0 ]]; then
        log_error "FINAL TEST FAILED: bwrap could not execute a simple command like 'ls'."
        log_error "This confirms the issue is with the 'bwrap' installation or system configuration, not our script."
        log_error "Recommendation: Fall back to the original chroot/namespace sandbox method."
    else
        log_info "FINAL TEST SUCCEEDED: bwrap itself works."
        log_info "This means the problem is a subtle incompatibility with the Wine/Proton executable."
    fi

    return $exit_code
}

main "$@"