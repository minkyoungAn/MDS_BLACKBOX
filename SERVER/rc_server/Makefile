obj-m := ultrasonic.o

KDIR := /work/linux

CROSS_COMPILE=arm-none-linux-gnueabi-

KDIR	:= /work/linux/

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	$(CROSS_COMPILE)gcc -o rc_server rc_server.c th_ultra.c pwm_app.c -lpthread


install:
	cp *.ko /nfsroot/root/
	cp rc_server /nfsroot/root
	cp ultrasonic.ko /nfsroot/root

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 
	rm -rf example
	rm -rf *.o
