obj-m += barrierlayer_kernel.o
CC = gcc
EXTRA_CFLAGS += -Wno-unused-parameter -Wno-unused-variable
CONFIG_STACK_VALIDATION := n

# Configurações do kernel
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Flags de compilação
ccflags-y := -DDEBUG -g -O2 -Wall -Wextra

# Targets
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod.c *.mod *.order *.symvers

install: all
	sudo insmod barrierlayer_kernel.ko
	@echo "BarrierLayer kernel module loaded successfully!"
	@echo "Check logs with: sudo cat /proc/barrierlayer"
	@echo "Monitor kernel logs with: sudo dmesg | tail -f"

uninstall:
	sudo rmmod barrierlayer_kernel
	@echo "BarrierLayer kernel module unloaded successfully!"

reload: uninstall install

status:
	@echo "=== BarrierLayer Kernel Module Status ==="
	@lsmod | grep barrierlayer || echo "Module not loaded"
	@echo ""
	@echo "=== Recent Kernel Messages ==="
	@dmesg | grep BarrierLayer | tail -10 || echo "No recent messages"
	@echo ""
	@echo "=== Log Entries ==="
	@if [ -f /proc/barrierlayer ]; then \
		echo "Log file exists. First 20 lines:"; \
		sudo head -20 /proc/barrierlayer; \
	else \
		echo "Log file not available"; \
	fi

logs:
	@echo "=== BarrierLayer Detailed Logs ==="
	@sudo cat /proc/barrierlayer 2>/dev/null || echo "Logs not available (module not loaded?)"

monitor:
	@echo "=== Monitoring BarrierLayer Activity (Ctrl+C to stop) ==="
	@while true; do \
		clear; \
		echo "=== BarrierLayer Real-time Monitor ==="; \
		echo "Time: $$(date)"; \
		echo ""; \
		sudo tail -50 /proc/barrierlayer 2>/dev/null || echo "No logs available"; \
		sleep 2; \
	done

help:
	@echo "BarrierLayer Kernel Module Build System"
	@echo "======================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build the kernel module"
	@echo "  clean     - Clean build files"
	@echo "  install   - Load the kernel module"
	@echo "  uninstall - Unload the kernel module"
	@echo "  reload    - Unload and reload the module"
	@echo "  status    - Show module status and recent logs"
	@echo "  logs      - Show all current logs"
	@echo "  monitor   - Real-time log monitoring"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make all install    - Build and load module"
	@echo "  make status         - Check if module is working"
	@echo "  make logs           - View all intercepted calls"
	@echo "  make monitor        - Watch activity in real-time"

.PHONY: all clean install uninstall reload status logs monitor help