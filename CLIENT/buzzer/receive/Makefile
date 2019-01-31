obj-m := buzzer.o

KDIR := /work/linux

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	arm-none-linux-gnueabi-gcc -o buzzer main.c -lpthread

	cp buzzer.ko /nfsroot/root
	cp buzzer /nfsroot/root
clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 

main:
	arm-none-linux-gnueabi-gcc -o buzzer main.c -lpthread

install:
	cp buzzer.ko /nfsroot/root
	cp buzzer /nfsroot/root
