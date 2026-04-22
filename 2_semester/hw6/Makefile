# Generic kernel module Makefile (kbuild)
obj-m      := kwallet.o
KDIR       ?= /lib/modules/$(shell uname -r)/build
PWD        := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install::
	sudo insmod kwallet.ko
	# The module creates /proc/kwallet with mode 0666, so no extra chmod needed.

deinstall::
	sudo rmmod kwallet

clean::
	sudo rmmod kwallet || true
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *~

help::
	@echo "make        – build module"
	@echo "make install  – load module (creates /proc/kwallet)"
	@echo "make deinstall– remove module"
	@echo "make clean  – clean build files"

PHONY_EXTRA: install deinstall clean
