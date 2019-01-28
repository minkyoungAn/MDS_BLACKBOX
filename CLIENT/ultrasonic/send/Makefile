obj-m := ultrasonic.o

KDIR := /work/linux

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 

main:
	arm-none-linux-gnueabi-gcc -o ultrasonic main.c -lpthread

install:
	cp ultrasonic.ko /nfsroot/root
	cp ultrasonic /nfsroot/root
