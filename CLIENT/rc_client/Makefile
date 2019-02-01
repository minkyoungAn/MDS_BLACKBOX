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


ifeq ("$(ARCH)", "arm")
	CROSS_COMPILE=arm-none-linux-gnueabi-
endif

KDIR := /work/linux

allc:
	$(CROSS_COMPILE)gcc -o example example.c   `sdl-config-$(ARCH) --cflags --libs`   -lSDL -lSDL_ttf -lSDL_image -lpthread 

allcpp:
	$(CROSS_COMPILE)g++ -o example example.cpp `sdl-config-$(ARCH) --cflags --libs`   -lSDL -lSDL_ttf -lSDL_image  
install:
	cp  example  /nfsroot/root

clean:
	rm -rf example
	rm -rf *.o
 
