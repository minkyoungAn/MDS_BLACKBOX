# this is Makefile for SDL, SDL_ttf application
#

#
# sdl-config  is  at  /usr/bin 
#   for x86 application,
#	modify sdl-config,  prefix=/usr

#   for arm application,
#	modify sdl-config,  prefix=/usr/local/arm/4.4.1/arm-none-linux-gnueabi
#

#
#  $  export ARCH=arm ; make
#  $  export ARCH=x86 ; make
#

obj-m := buzzer.o
ARCH=arm
CROSS_COMPILE=arm-none-linux-gnueabi-

KDIR := /work/linux

allc:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	$(CROSS_COMPILE)gcc -o rc_client rc_client.c th_buzzer.c  `sdl-config-$(ARCH) --cflags --libs`   -lSDL -lSDL_ttf -lSDL_image -lpthread -lrt

allcpp:
	$(CROSS_COMPILE)g++ -o rc_client rc_client.cpp `sdl-config-$(ARCH) --cflags --libs`   -lSDL -lSDL_ttf -lSDL_image  
install:
	cp rc_client /nfsroot/root
	cp buzzer.ko /nfsroot/root

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 
	rm -rf rc_client
	rm -rf *.o

