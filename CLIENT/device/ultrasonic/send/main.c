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

static int sig_flag = 0 ;

void *ultrasonic_fun(void *parameter)
{
	int retn;
	int fd;
	int flag = 0;
	char buf[10] = {0};
	char ch;
	volatile int i;
	char a;
	int cnt=0;

	fd = open("/dev/ultrasonic",O_RDWR);
	//printf("fd = %d\n", fd);
	if(fd<0){
		perror("/dev/ultrasonic error");
		exit(-1);
	}else
	{
		printf("ultrasonic has been detected ...\n");
	}
	
	while(1)
	{
		read(fd,buf,2);
		printf(" main read buf[1] : %d \n",buf[1]);		
		
		if(buf[1]==0)
		{
			//warning
			sig_flag=1;
			// tcp send
			
		}
		else sig_flag=0;

	}

	
	return 0;
}


int main(void)
{

	pthread_t ultrasonic_th;

	pthread_create(&ultrasonic_th,NULL,&ultrasonic_fun,NULL);

	pthread_join(ultrasonic_th,NULL);

	return 0;
}
