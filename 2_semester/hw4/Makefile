# Generic kernel module Makefile (kbuild)
obj-m      := kwallet_sys.o kwallet_proc.o
KDIR       ?= /lib/modules/$(shell uname -r)/build
PWD        := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install::
	sudo insmod kwallet_sys.ko
	sudo insmod kwallet_proc.ko
	# The module creates /proc/kwallet with mode 0666, so no extra chmod needed.

deinstall::
	-sudo rmmod kwallet_sys
	-sudo rmmod kwallet_proc

clean::
	-sudo rmmod kwallet_sys
	-sudo rmmod kwallet_proc
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *~

help::
	@echo "make          – build both modules (kwallet_sys.ko, kwallet_proc.ko)"
	@echo "make install  – load both modules (creates /proc/kwallet)"
	@echo "make deinstall– remove both modules"
	@echo "make clean    – clean build files and unload modules if loaded"

PHONY_EXTRA: install deinstall clean
