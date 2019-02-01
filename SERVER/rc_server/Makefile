CROSS_COMPILE=arm-none-linux-gnueabi-

all:
	$(CROSS_COMPILE)gcc -o rc_server rc_server.c

install:
	cp rc_server /nfsroot/root

clean:
	rm -rf example
	rm -rf *.o