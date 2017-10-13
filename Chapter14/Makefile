obj-m := gpio-legacy-dt-module.o gpio-descriptor-module.o  gpio-legacy-module.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules
install: modules_install

modules modules_install help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
