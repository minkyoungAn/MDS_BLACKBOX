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

#define MS	1000000
pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;


int main(void)
{
	int fd;
	int retn_write;
	int retn_read;
	int cmd;
	int flag = 0;
	    
	char dev_name[1024];	

	struct pwm_pulse lpwm_pulse;
	struct pwm_pulse rpwm_pulse;


	bzero( dev_name, sizeof( dev_name ));
	sprintf( dev_name, "/dev/%s", DEV_PWM_NAME );
	mknod( dev_name, S_IFCHR|S_IRWXU|S_IRWXG, MKDEV( DEV_PWM_MAJOR, 0 ));
    printf("Make Device file(%s)\n", dev_name );





	/*write buffer*/
	char buf_write[100] = "write...\n";
	char buf_read[100] = {0};

	/*open DD*/
	fd = open("/dev/PWM",O_RDWR);
	printf("fd = %d\n",fd);

	if (fd<0){
		perror("/dev/PWM error");
		exit(-1);
	}
	else
		printf("PWM has been detected...\n");
	
	/*buffer read,write*/
//	retn_write = write( fd, buf_write, 10);//10byte write
//	printf("\n`Size of written data : %s\n",buf_write);
//	retn_read = read(fd, buf_read, 20);//20byte read
//	printf("\ndata : %s\n",buf_read);
	
	/*ioctl*/
//	getchar();
	cmd = FRONT;
	while(1){
		switch (cmd){
			case FRONT : 
			    lpwm_pulse.width = (10*MS);
			    lpwm_pulse.period = (1*MS);
			    ioctl(fd, LEFTPWM, &lpwm_pulse);
			    printf("Dutyf: %lu, Periodf: %lu\n", lpwm_pulse.width, lpwm_pulse.period );
			    printf("front1 : %d\n", cmd);
				rpwm_pulse.width = (1*MS);
			    rpwm_pulse.period = (10*MS);
			    ioctl(fd, RIGHTPWM, &rpwm_pulse);
			    printf("Dutyf: %lu, Periodf: %lu\n", rpwm_pulse.width, rpwm_pulse.period );
				printf("front2 : %d\n", cmd);
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
			    printf("Dutyl: %lu, Periodl: %lu\n", lpwm_pulse.width, lpwm_pulse.period );
				printf("left1 : %d\n", cmd);    
				rpwm_pulse.width = (7*MS);
			    rpwm_pulse.period = (10*MS);
			    ioctl(fd, RIGHTPWM, &rpwm_pulse);
			    printf("Dutyl: %lu, Periodl: %lu\n", rpwm_pulse.width, rpwm_pulse.period );
				printf("left2 : %d\n", cmd);
			    ioctl(fd,LEFT);
			break;

			case RIGHT :
			    lpwm_pulse.width = (7*MS);
			    lpwm_pulse.period = (10*MS);
			    ioctl(fd, LEFTPWM, &lpwm_pulse);
			    printf("Dutyr: %lu, Periodr: %lu\n", lpwm_pulse.width, lpwm_pulse.period );
				printf("right1 : %d\n", cmd);
			    rpwm_pulse.width = (2*MS);
			    rpwm_pulse.period = (10*MS);
			    ioctl(fd, RIGHTPWM, &rpwm_pulse);
			    printf("Dutyr: %lu, Periodr: %lu\n", rpwm_pulse.width, rpwm_pulse.period );
				printf("right2 : %d\n", cmd);
			    ioctl(fd,RIGHT);
			break;

			case STOP :
				printf("right2 : %d\n", cmd);
			    ioctl(fd,STOP);
			break;
			default : break;
		}	
	}	
	/*device release*/
	close(fd);

	return 0;
}

