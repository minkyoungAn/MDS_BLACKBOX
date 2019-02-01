/*
*	Filename : pwm_dev_app.c
*	Title : Pwm Device Application
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
#include "pwm_dev.h"
#include "moter_app.h"

#define MS	1000000
//pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;

void moter_mknod(void)
{
	char dev_name[1024];

	bzero( dev_name, sizeof( dev_name ));
	sprintf( dev_name, "/dev/%s", DEV_PWM_NAME );
	mknod( dev_name, S_IFCHR|S_IRWXU|S_IRWXG, MKDEV( DEV_PWM_MAJOR, 0 ));
    printf("Make Device file(%s)\n", dev_name );

    return;
}

void *moter_func(int cmd)
{
	int fd;
	int retn_write;
	int retn_read;
	int flag = 0;
	    	
	struct pwm_pulse lpwm_pulse;
	struct pwm_pulse rpwm_pulse;

	/*open DD*/
	fd = open("/dev/PWM",O_RDWR);
	printf("fd = %d\n",fd);

	if (fd<0){
		perror("/dev/PWM error");
		exit(-1);
	}
	else
		printf("PWM has been detected...\n");
	
	/*ioctl*/
	while(1){
		switch (cmd){
			case FRONT : 
			    lpwm_pulse.width = (10*MS);
			    lpwm_pulse.period = (1*MS);
			    ioctl(fd, LEFTPWM, &lpwm_pulse);
				rpwm_pulse.width = (1*MS);
			    rpwm_pulse.period = (10*MS);
			    ioctl(fd, RIGHTPWM, &rpwm_pulse);
			    ioctl(fd,FRONT);
				break;		
	
			case BACK :
				lpwm_pulse.width = (10*MS);
			    lpwm_pulse.period = (1*MS);
			    ioctl(fd, LEFTPWM, &lpwm_pulse);
			    rpwm_pulse.width = (1*MS);
			    rpwm_pulse.period = (10*MS);
			    ioctl(fd, RIGHTPWM, &rpwm_pulse);
			    //printf("Dutyf: %lu, Periodf: %lu\n", rpwm_pulse.width, rpwm_pulse.period );
			    ioctl(fd,BACK);
				break;		

				
			case LEFT :
			    lpwm_pulse.width = (2*MS);
			    lpwm_pulse.period = (10*MS);
			    ioctl(fd, LEFTPWM, &lpwm_pulse);
				rpwm_pulse.width = (7*MS);
			    rpwm_pulse.period = (10*MS);
			    ioctl(fd, RIGHTPWM, &rpwm_pulse);
			    ioctl(fd,LEFT);
			break;

			case RIGHT :
			    lpwm_pulse.width = (7*MS);
			    lpwm_pulse.period = (10*MS);
			    ioctl(fd, LEFTPWM, &lpwm_pulse);
			    rpwm_pulse.width = (2*MS);
			    rpwm_pulse.period = (10*MS);
			    ioctl(fd, RIGHTPWM, &rpwm_pulse);
			    ioctl(fd,RIGHT);
			break;

			case STOP :
			    ioctl(fd,STOP);
			break;
			default : break;
		}	
	}	
	/*device release*/
	close(fd);

	return 0;
}

