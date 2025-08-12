# INSTALLATION GUIDE FOR BARRIERLAYER KERNEL-MODE

This guide provides concise instructions for compiling and installing the BarrierLayer Kernel-Mode components.

## 1. Prerequisites

Before you begin, ensure you have the following installed on your system:

*   **Linux Kernel Headers:** Must match your currently running kernel version. You can usually install them via your distribution's package manager (e.g., `sudo apt install linux-headers-$(uname -r)` on Debian/Ubuntu).
*   **`gcc`:** The GNU C Compiler.
*   **`make`:** The GNU Make utility.
*   **`sudo`:** For elevated privileges required during installation.

## 2. Compilation

Navigate to the root directory of the BarrierLayer project (where `Makefile.advanced` is located) and compile all components:

```bash
make -f Makefile.advanced all
```

This command will compile:

*   The kernel module: `kernel/barrierlayer_kernel_advanced.ko`
*   The user-mode launcher: `bin/stealth_launcher`
*   The user-mode injection tool: `bin/advanced_injection`

## 3. Installation

To install the compiled components to standard system paths, run the following command from the project root:

```bash
sudo make -f Makefile.advanced install
```

This will:

*   Install `stealth_launcher` and `advanced_injection` to `/usr/local/bin/`.
*   Install `barrierlayer_kernel_advanced.ko` to `/lib/modules/$(uname -r)/extra/` and update kernel module dependencies.
*   Create a configuration file at `/etc/barrierlayer/config`.
*   Create helper scripts `barrierlayer-load` and `barrierlayer-unload` in `/usr/local/bin/`.

## 4. Uninstallation

To remove the installed BarrierLayer Kernel-Mode components from your system, run:

```bash
sudo make -f Makefile.advanced uninstall
```

This will unload the module (if loaded), remove the binaries, kernel module, configuration, and helper scripts.

## 5. Verification

After compilation, you can run basic tests:

```bash
make -f Makefile.advanced test
```

After installation, you can verify the module loading/unloading:

```bash
# Load the kernel module
sudo barrierlayer-load
lsmod | grep barrierlayer_kernel_advanced
# Expected: module listed

# Unload the module
sudo barrierlayer-unload
lsmod | grep barrierlayer_kernel_advanced
# Expected: no output
```