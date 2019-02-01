CROSS_COMPILE=arm-none-linux-gnueabi-

KDIR	:= /work/linux/

all:
	$(CROSS_COMPILE)gcc -o rc_server rc_server.c pwm_app.c
	make -C $(KDIR) SUBDIRS=$(PWD) modules

install:
	cp *.ko /nfsroot/root/
	cp rc_server /nfsroot/root

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -rf example
	rm -rf *.o
