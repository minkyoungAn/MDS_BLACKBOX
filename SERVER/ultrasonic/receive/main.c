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

#include "sk.h"

static int sig_flag = 0 ;

void *ultra_fun(void *parameter)
{
	int retn;
	int fd;
	int flag = 0;
	char buf[10] = {0};
	char ch;
	volatile int i;
	char a;
	int cnt=0;

	fd = open("/dev/SK",O_RDWR);
	printf("fd = %d\n", fd);
	if(fd<0){
		perror("/dev/SK error");
		exit(-1);
	}else
	{
		printf("SK has been detected ...\n");
	}
	/*
	char buf[2];
	int fd;
	fd = open("/dev/SK" , O_RDWR ) ;
	if( fd < 0 )
	{
		perror("/dev/SK error\n" );
		exit(-1);
	}

*/
	
	while(1)
	{
		 read(fd,buf, 1);
		 if ( read < 0 ) 
		 {
			printf("read error \n");	//error !!!
		 }
		
		if(buf[0]==2)
		{

			write(fd,"b",1);
			if(buf[1]==1){
				ioctl(fd,GO_STRAIGHT,flag);
			}else if(buf[1]==0)
			{
				ioctl(fd,STOP,flag);
			}
		}
		else if(buf[0]==1)
		{
			ioctl(fd,STOP,flag);
		}
		else if(buf[0]==0)
		{
			ioctl(fd,GO_BACK,flag);
		}

		if(cnt==0)
		{	cnt++;
			ioctl(fd,GO_STRAIGHT,flag);
		}
		if ( sig_flag == 1 )        
		{
			printf("hallo~~!~!");

//			pwm_duty.pulse_width = 150000;
//			pwm_duty.period = 350000;
			write(fd,"c",1);		

			ioctl(fd,STOP,flag);
			sleep(2);			
			//write(fd,"b",1);
			sig_flag = 0;
		}
	//	printf("-0-----------------------------------------------\n");
	
		for ( i = 0 ; i <0x1fffff; i++ );
		for(i=0;i<0xfffff;i++);
		for(i=0;i<0xfffff;i++);
		
	}
	return 0;
}

void *blue_fun(void *parameter)
{
	int blue_fd, sk_fd;
	int cnt;
	char buf[5]; 
	memset(buf , '\0' , 5 );
	blue_fd = open("/dev/ttySAC2",O_RDWR|O_NOCTTY);

	struct termios newtio,oldtio;

	tcgetattr(blue_fd , &oldtio);

	memset(&newtio,0,sizeof(newtio));
	newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD ;
	newtio.c_iflag = IGNPAR | ICRNL ;
	newtio.c_oflag = 0 ;
	newtio.c_lflag = 0 ;
	newtio.c_cc[VTIME] = 100 ;
	newtio.c_cc[VMIN] = 2 ;

	tcflush( blue_fd , TCIFLUSH);
	tcsetattr( blue_fd , TCSANOW, &newtio);





	while(1)
	{
		tcflush( blue_fd , TCIFLUSH);
		cnt=read( blue_fd ,buf, 3);
		if( !strcmp(buf, "XX")  )
		{
			printf("i got it!!!~~%s\n",buf);
			//mutax on
			sig_flag = 1;
			// do something
		}
		else
	  		memset(buf , '\0' , 5 );
		printf("IN WHILE~~%s\n",buf);
		 
	}



//	tcsetattr(blue_fd,TCSANOW,&oldtio);
//	close(blue_fd);

	return 0;

}



int main(void)
{

	pthread_t ultra_th,bluetooth_th ;

	pthread_create(&ultra_th,NULL,&ultra_fun,NULL);
	pthread_create(&bluetooth_th,NULL,&blue_fun,NULL);

	pthread_join(ultra_th,NULL);
	pthread_join(bluetooth_th,NULL);

	return 0;
}
