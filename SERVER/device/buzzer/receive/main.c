#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> 
#include <pthread.h> 
#include <signal.h>
#include <errno.h>
//////////////////////////////////////////////
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termio.h>
///////////////////////////////////////////////

#include "buzzer.h"

static int sig_flag = 0;

void *buzzer_fun(void *parameter)
{
	int fd;
	int flag = 0;
	volatile int i;

	fd = open("/dev/buzzer",O_RDWR);
	printf("fd = %d\n", fd);
	if(fd<0){
		perror("/dev/buzzer error");
		exit(-1);
	}else
	{
		printf("buzzer has been detected ...\n");
	}

	//while(1)
	int j = 0;
	for (j = 0; j < 3; j++)
	{
		// tcp receive
		sig_flag = 1;

		if ( sig_flag == 1 )        
		{

			write(fd,"c",1);		

			ioctl(fd,BEEP,sig_flag);
			sleep(0.9);			
			sig_flag = 0;
		}
		
	}


	return 0;
}


int main(void)
{

	pthread_t buzzer_th;

	pthread_create(&buzzer_th,NULL,&buzzer_fun,NULL);
	
	pthread_join(buzzer_th,NULL);
	

	return 0;
}
