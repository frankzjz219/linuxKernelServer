KERNELDIR := /root/study/LinuxDriver/Linux_5_7_9Kernel/linux-5.7.9
CURRENT_PATH := $(shell pwd)
obj-m := kernelServer.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean