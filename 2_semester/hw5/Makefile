# Generic kernel module Makefile (kbuild)
SHELL      := /bin/bash
obj-m      := kwallet.o
KDIR       ?= /lib/modules/$(shell uname -r)/build
PWD        := $(shell pwd)


compile:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc kwallet_cli.c -o kwallet_cli -Wall

install:: clean | compile
	sudo insmod kwallet.ko
	sudo chmod 666 /dev/kwallet

deinstall::
	sudo rmmod kwallet
	sudo rm -f /dev/kwallet

remove_artifacts:
	$(MAKE) -C $(KDIR) M=$(PWD) clean || true

clean:: remove_artifacts
	sudo rmmod kwallet || true
	rm -f *~ || true

help::
	@echo "make        – build module"
	@echo "make install  – load module & create /dev/kwallet"
	@echo "make deinstall– remove module"
	@echo "make clean  – clean build files"

all: install

PHONY_EXTRA: install deinstall clean
