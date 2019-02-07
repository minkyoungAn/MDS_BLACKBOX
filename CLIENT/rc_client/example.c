#include <SDL/SDL.h>
#include <SDL/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "th_buzzer.h"

#define MAXLINE 30

void* mplayer_stream_thread(void* data)
{
    system("mplayer -screenw 160 -screenh 120 -demuxer mpeg4es rtp://@192.168.1.159:7000");
    pthread_exit(NULL);
}

void mplayer_stream_thread_create(pthread_t mplayer_stream_t, int connect_server)
{	
	if (connect_server == 1)
	{
		pthread_create(&mplayer_stream_t, 0, mplayer_stream_thread, NULL);
	}	
}


void* tcp_read_thread(void* arg)
{
	int socket = *((int*)arg);
	char buf[MAXLINE];
	pthread_t th_buzzer;
	int size;
	int buzzersig;
	int cnt;
	int fd;
	
	#if 1
	printf("tcp_read_thread start socket=%d\n",socket);
	
	fd = open("/dev/buzzer",O_RDWR); //open buzzer
	
	while(1)
	{
		size = read(socket, buf, MAXLINE);
		
		if (size > 0)
		{
			switch(buf[0])
			{
				case 6:
					cnt++;				
					printf("buzzer activate!!!\n");
					buzzersig = BUZZER_SIG;
					#if 0
					if (pthread_create(&th_buzzer, NULL, &buzzer_func, (void *)&buzzersig) != 0) {
						puts("buzzer pthread_create() error!");    
						exit(1);
					}
					#else
					if (cnt >= 50)
					{
						buzzer_func(&buzzersig,fd);
						cnt = 0;
					}				
					#endif
					break;
	
				default:
					break;
			}
		}
	}
	
	close(fd);
	#endif

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

    while(1){
        if( (num/deg) > 0)
            cnt++;
        else
            break;
        deg *= radix;
    }
    deg /=radix; 

    for(i=0; i<cnt; i++)    { 
        *(str+i) = num/deg + '0';
        num -= ((num/deg) * deg);
        deg /=radix;
    }
    *(str+i) = '\0';
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

#if 1
	while (1)
	{
		int result;
		
		result = connect(server_sockfd, (struct sockaddr*)&serveraddr, client_len);
		if (result == 0) {			
			printf("connect success!!!!!!! server_sockfd = %d\n",server_sockfd);
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
#endif    
	
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
	Xbutton = SDL_LoadBMP("image/Xbutton.bmp");
	Stop = SDL_LoadBMP("image/Stop.bmp");
	Name = SDL_LoadBMP("image/Name.bmp");
	Logo = SDL_LoadBMP("image/Logo.bmp");

	SDL_Rect dstrect_Up	= {280, 25, 65, 65};
	SDL_Rect dstrect_Down	= {280, 185, 65, 65};
	SDL_Rect dstrect_Right	= {360, 105, 65, 65};
	SDL_Rect dstrect_Left	= {200, 105, 65, 65};
	SDL_Rect dstrect_button = {460, 0, 20, 20};
	SDL_Rect dstrect_stop   = {280, 105, 130, 65};
	SDL_Rect dstrect_name   = {10 , 230, 113, 35};
	SDL_Rect dstrect_logo   = {10 , 190, 120, 35};


	SDL_BlitSurface(main, NULL, screen, NULL); // 넣기
	SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
	SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
	SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
	SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
	SDL_BlitSurface(Xbutton, NULL, screen, &dstrect_button);
	SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
	SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
	SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
	SDL_Flip(screen); // 갱신

	mplayer_stream_thread_create(mplayer_stream_t,connect_server);
	tcp_read_thread_create(tcp_read_t, connect_server, &server_sockfd );
	

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
			SDL_BlitSurface(Xbutton, NULL, screen, &dstrect_button);
			SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
			SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
			SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
			SDL_Flip(screen);
       
			memset(buf, 0x00, MAXLINE);
			strcpy(buf, "1");
			printf("pressed %s button\n", buf);

			write(server_sockfd, buf, MAXLINE);
			printf("%d, %d \n", event.motion.x, event.motion.y);
		
			}

			else if((event.motion.x >=280) && (event.motion.x <=355) && (event.motion.y >=185) && (event.motion.y <=260)) // Down button click.
			{
			SDL_BlitSurface(main, NULL, screen, NULL);
			SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
			SDL_BlitSurface(Down2, NULL, screen, &dstrect_Down);
			SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
			SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
			SDL_BlitSurface(Xbutton, NULL, screen, &dstrect_button);
			SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
			SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
			SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
			SDL_Flip(screen);
			
			memset(buf, 0x00, MAXLINE);
			strcpy(buf, "2");
			printf("pressed %s button\n", buf);

			write(server_sockfd, buf, MAXLINE);
			printf("%d, %d \n", event.motion.x, event.motion.y);
		
			}

			else if((event.motion.x >=360) && (event.motion.x <= 435) && (event.motion.y >= 105) && (event.motion.y <= 180)) // Right button click.
			{
			SDL_BlitSurface(main, NULL, screen, NULL);
			SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
			SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
			SDL_BlitSurface(Right2, NULL, screen, &dstrect_Right);
			SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
			SDL_BlitSurface(Xbutton, NULL, screen, &dstrect_button);
			SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
			SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
			SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
			SDL_Flip(screen);

			memset(buf, 0x00, MAXLINE);
			strcpy(buf, "3");
			printf("pressed %s button\n", buf);

			write(server_sockfd, buf, MAXLINE);
			printf("%d, %d \n", event.motion.x, event.motion.y);
			}

			else if((event.motion.x >=200) && (event.motion.x <=270) && (event.motion.y >=105) && (event.motion.y <=175))  // Left button click.
			{
			SDL_BlitSurface(main, NULL, screen, NULL);
			SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
			SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
			SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
			SDL_BlitSurface(Left2, NULL, screen, &dstrect_Left);
			SDL_BlitSurface(Xbutton, NULL, screen, &dstrect_button);
			SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
			SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
			SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
			SDL_Flip(screen);

			memset(buf, 0x00, MAXLINE);
			strcpy(buf, "4");
			printf("pressed %s button\n", buf);

			write(server_sockfd, buf, MAXLINE);
			printf("%d, %d \n", event.motion.x, event.motion.y);
			}

			else if((event.motion.x >=280) && (event.motion.x <=355) && (event.motion.y >=105) && (event.motion.y <=180)) // Stop button click.
			{
			SDL_BlitSurface(main, NULL, screen, NULL);
			SDL_BlitSurface(Up, NULL, screen, &dstrect_Up);
			SDL_BlitSurface(Down, NULL, screen, &dstrect_Down);
			SDL_BlitSurface(Right, NULL, screen, &dstrect_Right);
			SDL_BlitSurface(Left, NULL, screen, &dstrect_Left);
			SDL_BlitSurface(Xbutton, NULL, screen, &dstrect_button);
			SDL_BlitSurface(Stop, NULL, screen, &dstrect_stop);
			SDL_BlitSurface(Name, NULL, screen, &dstrect_name);
			SDL_BlitSurface(Logo, NULL, screen, &dstrect_logo);
			SDL_Flip(screen);

			memset(buf, 0x00, MAXLINE);
			strcpy(buf, "5");
			printf("pressed %s button\n", buf);

			write(server_sockfd, buf, MAXLINE);
			printf("%d, %d \n", event.motion.x, event.motion.y);
			}
			
			

			if((event.motion.x >= 455) && (event.motion.y <=25))
			{
			SDL_BlitSurface(quitting, NULL, screen, NULL);
			SDL_Flip(screen);
			printf("program end\n");
			loop =1;
			}
			break;			
            }

        }
    }

    SDL_Quit();

    return 0;

}
