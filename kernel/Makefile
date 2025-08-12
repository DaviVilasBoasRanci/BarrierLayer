# Makefile for the BarrierLayer advanced kernel module

# This Makefile is called by the main Makefile.advanced

# Object file to build
obj-m += barrierlayer_kernel_advanced.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean