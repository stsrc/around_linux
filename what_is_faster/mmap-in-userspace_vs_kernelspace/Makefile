ifneq ($(KERNELRELEASE),)
	obj-m := driver.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	gcc -o userspace_app userspace.c --std=gnu99 -Wall -Wextra -g3
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -f *.o *.ko *.mod.c modules.order Module.symvers

endif
