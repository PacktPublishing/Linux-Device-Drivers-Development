obj-m := dedicated-workqueue.o hr-timer.o shared-workqueue.o std-timer.o tasklet.o user-invoke.o waitqueue.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules
install: modules_install

modules modules_install help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
