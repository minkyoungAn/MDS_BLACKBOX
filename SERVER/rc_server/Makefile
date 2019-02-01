obj-m := ultrasonic.o

KDIR := /work/linux

CROSS_COMPILE=arm-none-linux-gnueabi-

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	$(CROSS_COMPILE)gcc -o rc_server rc_server.c th_ultra.c -lpthread

install:
	cp rc_server /nfsroot/root
	cp ultrasonic.ko /nfsroot/root

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 
	rm -rf example
	rm -rf *.o