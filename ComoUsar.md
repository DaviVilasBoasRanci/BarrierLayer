# How to Use BarrierLayer

This guide details how to configure, compile, and run BarrierLayer to play your games.

## Step 1: Configuration

Before compiling, run the configuration script to set your preferences. It will ask which compatibility layer you want to use by default.

```bash
./configure.sh
```

You will have the following options:
- **Proton**: Recommended for Steam games. The script will try to detect your Proton installation.
- **Wine**: For Windows games or applications that are not from Steam.
- **Auto-Detect**: The launcher will try to decide the best option at runtime.

Your choice will be saved in the `config.mk` file.

## Step 2: Compilation

After configuring, you can compile the project. The main `Makefile` offers several targets:

- **Compile everything:**
  ```bash
  make all
  ```
  This command compiles user components (`barrierlayer_hook.so`, `test_runner`, etc.) and the kernel module (`barrierlayer_kernel.ko`).

- **Compile only user components:**
  ```bash
  make userspace
  ```

- **Compile only the kernel module:**
  ```bash
  make kernel
  ```

## Step 3: Running Applications

There are two main ways to use BarrierLayer.

### Method 1: Standalone Launcher

The `barrierlayer-launcher.sh` script is a smart and interactive launcher. It is ideal for testing executables outside of Steam.

-   **Force Proton usage:**
    ```bash
    ./barrierlayer-launcher.sh -p /path/to/your/game.exe
    ```

-   **Force Wine usage:**
    ```bash
    ./barrierlayer-launcher.sh -w /path/to/your/game.exe
    ```

### Method 2: Steam Integration (Recommended)

For Steam games like Splitgate, it is best to use the integration script directly in the game's launch options.

1.  In your Steam library, right-click on the game and go to **Properties...**
2.  In the **General** tab, find the **Launch Options** text box.
3.  Paste the following command, **using the absolute path** to the script:

    ```
    ENABLE_BARRIERLAYER=1 /home/davivbrdev/BarrierLayer/scripts/run_with_barrierlayer.sh %command%
    ```

4.  Close the properties and launch the game from Steam.

## Step 4: Analyzing the Output (Logs)

When a game is run with the Steam integration script (`run_with_barrierlayer.sh`), all hook output is redirected to a log file in the project root.

-   **Log File:** `barrierlayer_gui.log`

To monitor hook activity in real-time, open a terminal in the BarrierLayer folder and use the `tail` command:

```bash
tail -f barrierlayer_gui.log
```

This will show all intercepted functions, which is crucial for understanding game and anti-cheat behavior.
