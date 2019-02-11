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
#include <netinet/in.h>
#include <arpa/inet.h>


#include <sys/stat.h>	//mknod inner
#include <sys/types.h>

#include <pthread.h>
#include "led.h"
#include "pwm_dev.h"

void led_mknod(void)
{
	char dev_name[1024];

	bzero( dev_name, sizeof( dev_name ));
	sprintf( dev_name, "/dev/%s", DEV_PWM_NAME );
	mknod( dev_name, S_IFCHR|S_IRWXU|S_IRWXG, MKDEV( DEV_PWM_MAJOR, 0 ));
    printf("Make Device file(%s)\n", dev_name );
    return;
}



void *led_func(void *cmd)
{
	int fd;
	char buf[100] = "Write..\n";
	int flag = 0;

	fd = open("/dev/SK", O_RDWR);
	
	if (fd<0) {
        perror("/dev/SK error");
        exit(-1);
    }
	int res = LEFT;
    while(1){
		switch(res)
        {
            case LEFT:
				ioctl(fd, LEFT);
                break;
            case RIGHT:
				ioctl(fd, RIGHT);
                break;
            default:
                break;
        }
	}
	close(fd);
	
	return 0;
}