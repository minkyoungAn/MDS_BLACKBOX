#include <SDL/SDL.h>
#include <SDL/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "th_buzzer.h"

#define MAXLINE 30
//#define DUMP_DEBUG
#define STREAM_DUMP

#ifdef STREAM_DUMP
int dumping = 0;
#endif

timer_t buzzer_timer;
int buzzer_onoff;
int buzzer_fd;

void* mplayer_stream_thread(void* data)
{
	#ifdef STREAM_DUMP
	mkfifo("/root/mplayer_fifo",S_IRUSR|S_IWUSR); //make fifo for mplayer control
	system("mplayer -slave -input file=/root/mplayer_fifo -screenw 160 -screenh 120 -demuxer mpeg4es rtp://@192.168.1.159:7000");
	#else
    system("mplayer -screenw 160 -screenh 120 -demuxer mpeg4es rtp://@192.168.1.159:7000");
	#endif

    pthread_exit(NULL);
}

void mplayer_stream_thread_create(pthread_t mplayer_stream_t, int connect_server)
{	
	#ifndef DUMP_DEBUG
	if (connect_server == 1)
	#endif
	{		
		pthread_create(&mplayer_stream_t, 0, mplayer_stream_thread, NULL);
	}	
}

#ifdef STREAM_DUMP
void* mplayer_dump_thread(void *data)
{
	system("mplayer_cmd quit");
	sleep(1);
	system("mplayer -dumpstream -dumpfile /root/test.mp4 -slave -input file=/root/mplayer_fifo -screenw 160 -screenh 120 -demuxer mpeg4es rtp://@192.168.1.159:7000");

	pthread_exit(NULL);
}

void mplayer_close(void)
{
	int i;
	int pid;
	FILE *fd;
	char name[1024];
	char readbuf[1024];

	printf("%s\n",__func__);
	pid = getpid();

	for (i=0;i<1000;i++)
	{
		memset(name,0,1024);
		memset(readbuf,0,1024);

		sprintf(name, "/proc/%d/comm",pid);
		
		fd = fopen(name,"r");
		if(fd)
		{
			size_t size;
			size = fread(readbuf, sizeof(char), 7, fd);

			if (0 == strcmp(readbuf,"mplayer"))
			{
				printf("find dump process kill it\n");
				
				kill(pid,2);			

				fclose(fd);
				break;
			}

			fclose(fd);
        }

		pid++;
	}
}

void mplayer_dump_copy(vodi)
{
	time_t curr_time;
	char cp_buf[256];

	memset(cp_buf,0,256);

	time(&curr_time);
	sprintf(cp_buf,"cp /root/test.mp4 /mnt/sdcard/%ld.mp4",curr_time);
	system(cp_buf);
	system("rm /root/test.mp4");
}

void* mplayer_resume_thread(void *data)
{
	sleep(10);
	mplayer_close();
	sleep(3);
	mplayer_dump_copy();
	dumping = 0;
	system("mplayer -slave -input file=/root/mplayer_fifo -screenw 160 -screenh 120 -demuxer mpeg4es rtp://@192.168.1.159:7000");

	pthread_exit(NULL);
}
#endif

void* tcp_read_thread(void* arg)
{
	int socket = *((int*)arg);
	char buf[MAXLINE];
	int size;
	int buzzersig;
	int cnt;
	int fd;

#ifdef STREAM_DUMP
	pthread_t mplayer_dump;
	pthread_t mplayer_resume;
#endif

	printf("tcp_read_thread start socket=%d\n",socket);
	
	fd = open("/dev/buzzer",O_RDWR); //open buzzer
	buzzer_fd = fd;
	while(1)
	{
		size = read(socket, buf, MAXLINE);

		if (size > 0)
		{
			if (buf[0] == 6)
			{
				cnt++;				
				buzzersig = BUZZER_SIG;
				
				if (cnt >= 50)
				{
					#ifdef STREAM_DUMP
					if (dumping == 0)
					{
						dumping = 1;
						printf("dump start!!\n");
						pthread_create(&mplayer_dump, 0, mplayer_dump_thread, NULL);
						pthread_create(&mplayer_resume, 0, mplayer_resume_thread, NULL);
					}
					#endif
					buzzer_onoff = 1;
					cnt = 0;
				}
			}
		}
	}
	
	close(fd);

	pthread_exit(NULL);
}

void tcp_read_thread_create(pthread_t tcp_read_t, int connect_server, int *server_socket)
{
	if (connect_server ==1)
	{
		pthread_create(&tcp_read_t, 0, tcp_read_thread, (void*)server_socket);
	}
}

void itoa(int num, char *str){
    int i=0;
    int radix = 10;
    int deg=1;
    int cnt = 0;

    while(1)
	{
        if( (num/deg) > 0)
            cnt++;
        else
            break;
        deg *= radix;
    }
    deg /=radix; 

    for(i=0; i<cnt; i++) { 
        *(str+i) = num/deg + '0';
        num -= ((num/deg) * deg);
        deg /=radix;
    }
    *(str+i) = '\0';
}

void buzzer_timer_func(void)
{	
	if (buzzer_onoff == 1)
	{
		buzzer_onoff = 0;
		buzzer_func(buzzer_fd);
	}
}

void create_timer(void (*timer_func)(void), timer_t *timer_id,int sec)
{
	struct sigevent         te;  
    struct itimerspec       its;  
    struct sigaction        sa;  
    int sigNo = SIGRTMIN;  
   
    /* Set up signal handler. */  
    sa.sa_flags = SA_SIGINFO;  
    sa.sa_sigaction = timer_func;  
    sigemptyset(&sa.sa_mask);  
  
    if (sigaction(sigNo, &sa, NULL) == -1)  
    {  
        printf("sigaction error\n");
        return;
    }  
   
    /* Set and enable alarm */  
    te.sigev_notify = SIGEV_SIGNAL;  
    te.sigev_signo = sigNo;  
    te.sigev_value.sival_ptr = timer_func;  
    timer_create(CLOCK_REALTIME, &te, timer_id);  
   
    its.it_interval.tv_sec = sec;
    its.it_interval.tv_nsec = 0;  
    its.it_value.tv_sec = sec;
    
    its.it_value.tv_nsec = 0;
    timer_settime(*timer_id, 0, &its, NULL);	
}

int main(int argc, char *argv[])
{
    int loop = 0;
	int on_off =0;
	int retn;
	int flag = 0;
	struct sockaddr_in serveraddr;
	int server_sockfd;
	int client_len;
	char buf[MAXLINE];
	pthread_t mplayer_stream_t;
	pthread_t mplayer_dump_t;
	pthread_t tcp_read_t;
	int connect_server = 0;
	int result;

	// mknod buzzer
    buzzer_mknod();

	if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error ");
		return 1;
	}
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.158");
	serveraddr.sin_port = htons(6000);
	client_len = sizeof(serveraddr);   //client, serve

#ifndef DUMP_DEBUG
	while (1)
	{
		int result;
		
		result = connect(server_sockfd, (struct sockaddr*)&serveraddr, client_len);
		if (result == 0) {			
			printf("connect success! server_sockfd = %d\n",server_sockfd);
	    	connect_server =  1;
	    	break;	                
	    }
		else
	    {
			perror("connect error : ");
	        connect_server = 0;	    
	    	sleep(5);
	    }
	}
#endif //DUMP_DEBUG  
	
	printf("TTF_Init\n");

    SDL_Event event;
    SDL_Surface *screen, *main, *bmp, *quitting, *Up, *Down, *Left, *Right, *Up2, *Down2, *Left2, *Right2, *Xbutton, *Stop, *Name, *Logo;

	SDL_Surface* message = NULL;

	SDL_Color textColor = {255, 255, 255};

	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
 
	int time=0;
	time = SDL_GetTicks();
	char str[10];
	itoa(time, str);
  	screen = SDL_SetVideoMode(480, 272, 16, SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF);

    main = SDL_LoadBMP("image/Background.bmp");
	Up = SDL_LoadBMP("image/Up.bmp");
	Up2= SDL_LoadBMP("image/Up2.bmp");	
	Down = SDL_LoadBMP("image/Down.bmp");	
	Down2 = SDL_LoadBMP("image/Down2.bmp");	
	Left = SDL_LoadBMP("image/Left.bmp");	
	Left2 = SDL_LoadBMP("image/Left2.bmp");	
	Right = SDL_LoadBMP("image/Right.bmp");	
	Right2 = SDL_LoadBMP("image/Right2.bmp");	
	Stop = SDL_LoadBMP("image/Stop.bmp");
	Name = SDL_LoadBMP("image/Name.bmp");
	Logo = SDL_LoadBMP("image/Logo.bmp");

	SDL_Rect dstrect_Up		= {280, 25, 65, 65};
	SDL_Rect dstrect_Down	= {280, 185, 65, 65};
	SDL_Rect dstrect_Right	= {360, 105, 65, 65};
	SDL_Rect dstrect_Left	= {200, 105, 65, 65};
	SDL_Rect dstrect_stop   = {280, 105, 130, 65};
	SDL_Rect dstrect_name   = {10 , 230, 113, 35};
	SDL_Rect dstrect_logo   = {10 , 190, 120, 35};


	SDL_BlitSurface(main, NULL, screen, NULL); // 넣기
	SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
	SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
	SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
	SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
	SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
	SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
	SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
	SDL_Flip(screen); // 갱신

	mplayer_stream_thread_create(mplayer_stream_t,connect_server);

	#ifndef DUMP_DEBUG
	tcp_read_thread_create(tcp_read_t, connect_server, &server_sockfd );
	#endif

	create_timer(buzzer_timer_func,&buzzer_timer,1); //buzzer timer, 1s

    while(!loop)
    {
        while(SDL_PollEvent(&event))
        {

            switch(event.type)
            {
                case SDL_QUIT:
                    loop = 1;
               		break;

				case SDL_MOUSEBUTTONDOWN:
					
					time++;
					itoa(time, str);
		
					if((event.motion.x >=280) && (event.motion.x <=355) && (event.motion.y >=25) && (event.motion.y <=100))  // Up button click.
					{
					SDL_BlitSurface(main, NULL, screen, NULL);
					SDL_BlitSurface(Up2, NULL, screen, &dstrect_Up);
					SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
					SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
					SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
					SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
					SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
					SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
					SDL_Flip(screen);
		       
					memset(buf, 0x00, MAXLINE);
					strcpy(buf, "1");
					printf("pressed %s button\n", buf);
		
					#ifndef DUMP_DEBUG
					write(server_sockfd, buf, MAXLINE);
					#endif

					printf("%d, %d \n", event.motion.x, event.motion.y);
				
					}
		
					else if((event.motion.x >=280) && (event.motion.x <=355) && (event.motion.y >=185) && (event.motion.y <=260)) // Down button click.
					{
					SDL_BlitSurface(main, NULL, screen, NULL);
					SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
					SDL_BlitSurface(Down2, NULL, screen, &dstrect_Down);
					SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
					SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
					SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
					SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
					SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
					SDL_Flip(screen);
					
					memset(buf, 0x00, MAXLINE);
					strcpy(buf, "2");
					printf("pressed %s button\n", buf);

					#ifndef DUMP_DEBUG		
					write(server_sockfd, buf, MAXLINE);
					#endif

					printf("%d, %d \n", event.motion.x, event.motion.y);
				
					}
		
					else if((event.motion.x >=360) && (event.motion.x <= 435) && (event.motion.y >= 105) && (event.motion.y <= 180)) // Right button click.
					{
					SDL_BlitSurface(main, NULL, screen, NULL);
					SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
					SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
					SDL_BlitSurface(Right2, NULL, screen, &dstrect_Right);
					SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
					SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
					SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
					SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
					SDL_Flip(screen);
		
					memset(buf, 0x00, MAXLINE);
					strcpy(buf, "3");
					printf("pressed %s button\n", buf);

					#ifndef DUMP_DEBUG		
					write(server_sockfd, buf, MAXLINE);
					#endif

					printf("%d, %d \n", event.motion.x, event.motion.y);
					}
		
					else if((event.motion.x >=200) && (event.motion.x <=270) && (event.motion.y >=105) && (event.motion.y <=175))  // Left button click.
					{
					SDL_BlitSurface(main, NULL, screen, NULL);
					SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
					SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
					SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
					SDL_BlitSurface(Left2, NULL, screen, &dstrect_Left);
					SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
					SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
					SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
					SDL_Flip(screen);
		
					memset(buf, 0x00, MAXLINE);
					strcpy(buf, "4");
					printf("pressed %s button\n", buf);

					#ifndef DUMP_DEBUG		
					write(server_sockfd, buf, MAXLINE);
					#endif

					printf("%d, %d \n", event.motion.x, event.motion.y);
					}
		
					else if((event.motion.x >=280) && (event.motion.x <=355) && (event.motion.y >=105) && (event.motion.y <=180)) // Stop button click.
					{
					SDL_BlitSurface(main, NULL, screen, NULL);
					SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
					SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
					SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
					SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
					SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
					SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
					SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
					SDL_Flip(screen);
		
					memset(buf, 0x00, MAXLINE);
					strcpy(buf, "5");
					printf("pressed %s button\n", buf);

					#ifndef DUMP_DEBUG		
					write(server_sockfd, buf, MAXLINE);
					#endif
					
					printf("%d, %d \n", event.motion.x, event.motion.y);
					}
					
					break;			
            }

        }
    }

    SDL_Quit();

    return 0;

}
