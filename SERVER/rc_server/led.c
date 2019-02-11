/*
*	Filename : led.c
*	Title :
*	Desc :
*
*
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termio.h>
#include <linux/ioctl.h>
#include <linux/kdev_t.h> // MKDEV

#include <sys/stat.h>	//mknod inner

#include <pthread.h>
#include "led.h"
#include "pwm_dev.h"
//pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;

void led_mknod(void)
{
	char dev_name[1024];

	bzero( dev_name, sizeof( dev_name ));
	sprintf( dev_name, "/dev/%s", DEV_SK_NAME );
	mknod( dev_name, S_IFCHR|S_IRWXU|S_IRWXG, MKDEV( DEV_SK_MAJOR, 0 ));
        printf("Make Device file(%s)\n", dev_name );
        return;
}



void *led_func(void *cmd)
{
	int fd;
	printf("cmd value : %d\n", *(int *)cmd);
	fd = open("/dev/SK", O_RDWR);
	
	if (fd<0) {
        perror("/dev/SK error");
        exit(-1);
    }
			
	switch(*(int *)cmd)
    {
        case LEFT:
        	printf("left led on\n");
			ioctl(fd, LEFT);
            break;
        case RIGHT:
        	printf("right led on\n");
			ioctl(fd, RIGHT);
            break;
        default:
            break;
	}
	
	close(fd);
	
	return 0;
}
