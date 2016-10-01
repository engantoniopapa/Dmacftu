# Comment/uncomment the following line to disable/enable debugging
#DEBUG=y

MODNAME= micro-benchmark_Band

ALL_SOURCE=$(shell echo *.c *.h)

# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
	DEBFLAGS = -Wall -Wstrict-prototypes -O -g -DMY_DEBUG -DDRIVER_NAME="\"$(MODNAME)\""
else
	DEBFLAGS = -Wall -Wstrict-prototypes -O2 -DDRIVER_NAME="\"$(MODNAME)\""
endif

EXTRA_CFLAGS += $(DEBFLAGS)

ifneq ($(KERNELRELEASE),)

obj-m:= $(MODNAME).o
$(MODNAME)-objs :=  micro_benchmark.o driverops.o core_benchmark.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD:= $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

tags: $(ALL_SOURCE)
	ctags --language-force=C $(ALL_SOURCE)

TAGS: $(ALL_SOURCE)
	ctags -e --language-force=C $(ALL_SOURCE)

.PHONY: clean

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers modules.order Module.markers
	rm -rf tags TAGS
