obj-m += sk.o

#KDIR := /media/sf_driver/linux-3.0.22/
KDIR := /work/linux

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean

install:
	cp sk.ko /nfs/root
