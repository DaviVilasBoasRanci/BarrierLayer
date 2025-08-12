# USAGE EXAMPLES FOR BARRIERLAYER KERNEL-MODE

This document provides practical examples for using the `scripts/advanced_launcher.sh` script and the installed `stealth_launcher` to manage and utilize the BarrierLayer Kernel-Mode components.

## 1. Basic Usage of `advanced_launcher.sh`

The `advanced_launcher.sh` script is the primary interface for interacting with the BarrierLayer kernel module and launching applications. Ensure it is executable:

```bash
chmod +x scripts/advanced_launcher.sh
```

### 1.1. Loading the Kernel Module

Before launching any application with BarrierLayer, you need to load the kernel module. This typically requires root privileges. You can use the helper script installed by `make install`:

```bash
sudo barrierlayer-load
```

Alternatively, using the `advanced_launcher.sh` script:

```bash
sudo scripts/advanced_launcher.sh --load-module
```

Upon successful loading, you should see a success message. You can verify its loaded status using `lsmod`:

```bash
lsmod | grep barrierlayer_kernel_advanced
# Expected output: barrierlayer_kernel_advanced <size> <count>
```

### 1.2. Unloading the Kernel Module

When you are finished using BarrierLayer, it's good practice to unload the kernel module. This also requires root privileges. You can use the helper script:

```bash
sudo barrierlayer-unload
```

Alternatively, using the `advanced_launcher.sh` script:

```bash
sudo scripts/advanced_launcher.sh --unload-module
```

You can verify its unloaded status:

```bash
lsmod | grep barrierlayer_kernel_advanced
# Expected output: (no output)
```

### 1.3. Launching an Application

To launch an application with BarrierLayer's kernel-mode features, use the `--launch` option followed by the executable path and its arguments. The kernel module must be loaded beforehand.

```bash
# Example: Launching a simple text editor (gedit)
sudo scripts/advanced_launcher.sh --launch /usr/bin/gedit

# Example: Launching Steam
sudo scripts/advanced_launcher.sh --launch /usr/bin/steam

# Example: Launching a Windows executable via Wine (assuming Wine is configured)
# The stealth_launcher will handle the communication with the kernel module.
sudo scripts/advanced_launcher.sh --launch wine /path/to/your/game.exe

# Example: Launching an application with arguments
sudo scripts/advanced_launcher.sh --launch /usr/bin/my_app --arg1 value1 --arg2
```

**Important:** The `stealth_launcher` (called by `advanced_launcher.sh --launch`) will handle the communication with the kernel module and the `advanced_injection` component to apply the necessary kernel-mode hooks and code injections. The target application will run with the enhanced anti-anti-cheat evasion capabilities.

### 1.4. Checking BarrierLayer Status

You can check the current status of the BarrierLayer components, including whether the kernel module is loaded and the device is available:

```bash
scripts/advanced_launcher.sh --status
```

### 1.5. Getting Help

For a quick overview of the available options, use the `--help` flag:

```bash
scripts/advanced_launcher.sh --help
```

## 2. Direct Usage of `stealth_launcher` (after `make install`)

Once installed to `/usr/local/bin/`, you can directly use `stealth_launcher`.

```bash
# Load kernel module first
sudo barrierlayer-load

# Launch an application
sudo stealth_launcher /usr/bin/gedit

# Unload kernel module when done
sudo barrierlayer-unload
```

Consult `stealth_launcher --help` for its specific options.