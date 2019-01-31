obj-m = pwm_dev.o

KDIR	:= /work/linux/

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	arm-none-linux-gnueabi-gcc -o pwm_app pwm_app.c
	cp *.ko pwm_app /nfsroot/root/
clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean
