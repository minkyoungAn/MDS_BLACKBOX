obj-m := ultrasonic.o pwm_dev.o


CROSS_COMPILE=arm-none-linux-gnueabi-

KDIR	:= /work/linux/

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	$(CROSS_COMPILE)gcc -o rc_server rc_server.c th_ultra.c pwm_app.c led.c -lpthread


install:
	cp *.ko /nfsroot/root/
	cp rc_server /nfsroot/root

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 
	rm -rf rc_server
	rm -rf *.o
