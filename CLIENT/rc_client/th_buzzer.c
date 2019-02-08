#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/major.h>
#include <linux/types.h>
#include <linux/kdev_t.h>

#include "th_buzzer.h"

void buzzer_mknod(void)
{
	char dev_name[1024];  

	bzero( dev_name, sizeof( dev_name ));
	sprintf( dev_name, "/dev/%s", DEV_BUZZER_NAME );
	mknod( dev_name, S_IFCHR|S_IRWXU|S_IRWXG, MKDEV( DEV_BUZZER_MAJOR, 0 ));
	printf("Make Device file(%s)\n", dev_name );

	return;
}

void *buzzer_func(int fd)
{
	ioctl(fd,BUZZER_SIG);

	return 0;
}

