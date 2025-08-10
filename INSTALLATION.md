# BarrierLayer Installation Guide

This document explains how to use the `install.sh` script to automatically install BarrierLayer on your Linux system. The script automates the build process, copies necessary files to standard locations, handles kernel module installation (if enabled), and sets up convenient shell aliases.

## Purpose of `install.sh`

The `install.sh` script is designed to provide a user-friendly and automated way to get BarrierLayer up and running. It consolidates multiple manual steps into a single command, ensuring that all components are placed correctly and configured for optimal use.

## What `install.sh` Installs and Where

The script installs BarrierLayer components into standard Linux system directories, primarily under `/usr/local/` for userspace components and `/lib/modules/` for the kernel module.

*   **Userspace Binaries:**
    *   `barrierlayer` (main executable)
    *   `barrierlayer_gui` (graphical user interface)
    *   `barrierlayer_hook.so` (hook library)
    These files are copied to:
    *   `/usr/local/bin/` (for executables)
    *   `/usr/local/lib/` (for shared libraries)
    These are conventional locations for software installed outside of your distribution's package manager.

*   **Documentation:**
    *   The entire `docs/` directory from the BarrierLayer source is copied to `/usr/local/share/barrierlayer/`.
    This provides easy access to project documentation after installation.

*   **Kernel Module (if `BUILD_KERNEL=Y` in `config.mk`):
    *   The compiled kernel module (`kernel/barrierlayer_minimal.ko`) is copied to `/lib/modules/$(uname -r)/extra/`.
    This is the standard location for out-of-tree kernel modules.
    *   A configuration file (`/etc/modules-load.d/barrierlayer_minimal.conf`) is created to ensure the kernel module is loaded automatically every time your system boots.

*   **Shell Aliases:**
    *   Convenience aliases are added to your shell's configuration file (either `~/.bashrc` or `~/.zshrc`). These aliases provide quick access to BarrierLayer commands from your terminal:
        *   `barrierlayer`: Alias for the main executable.
        *   `barrierlayer-gui`: Alias for the graphical launcher.
        *   `barrierlayer-config`: Alias to run the interactive configuration script (`make configs`).
        *   `barrierlayer-uninstall`: Alias to easily uninstall BarrierLayer.

## How `install.sh` Works (The Installation Process)

1.  **Sudo Privileges Check:** The script first verifies that it's being run with `sudo` (root) privileges, which are necessary for system-wide installations.
2.  **Project Build:** It executes `make` in the BarrierLayer's root directory to compile all necessary components based on your `config.mk` settings.
3.  **Userspace Installation:** It runs `sudo make install`. This command, defined in the project's `Makefile`, handles copying the compiled userspace binaries and documentation to their respective `/usr/local/` destinations.
4.  **Kernel Module Handling:**
    *   If the kernel module was built (configured in `config.mk`), the script copies it to the appropriate kernel modules directory.
    *   It then runs `depmod -a` to update the kernel's module dependency database.
    *   It attempts to load the module immediately using `modprobe`.
    *   Finally, it configures the module for automatic loading on system boot by creating a configuration file in `/etc/modules-load.d/`.
5.  **Alias Creation:** The script appends the alias definitions to your user's shell configuration file (`.bashrc` or `.zshrc`).

## Safety and Best Practices

The `install.sh` script adheres to standard Linux installation practices for open-source software:

*   **Standard Locations:** Files are installed to conventional system directories (`/usr/local/`, `/lib/modules/`, `/etc/modules-load.d/`) used for software not managed by your distribution's package manager.
*   **No Malicious Actions:** The script is transparent in its operations and does not perform any actions beyond installing BarrierLayer components. It does not modify critical system files outside of these standard installation paths.
*   **Explicit Sudo:** All operations requiring root privileges are performed using `sudo`, which is a standard and secure way to manage system-wide changes.
*   **Easy Uninstallation:** The `barrierlayer-uninstall` alias is provided to simplify the removal of BarrierLayer components from your system if needed.

## How to Use `install.sh`

1.  **Navigate to the BarrierLayer root directory** in your terminal.
2.  **Ensure BarrierLayer is configured** as desired (e.g., `make configs`).
3.  **Execute the script with `sudo`**:
    ```bash
    sudo ./install.sh
    ```
4.  **Reload your shell configuration** (or restart your terminal) for the new aliases to take effect:
    ```bash
    source ~/.bashrc  # Or ~/.zshrc if you use Zsh
    ```

## Uninstallation

To uninstall BarrierLayer, simply run the provided alias:

```bash
barrierlayer-uninstall
```

This will remove the installed binaries, libraries, documentation, and kernel module configuration from your system. You may need to manually remove the alias lines from your shell RC file if you wish.
