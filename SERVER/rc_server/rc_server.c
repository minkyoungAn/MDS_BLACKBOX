#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "th_ultra.h"
#include "moter_app.h"

#define SERV_TCP_PORT   6000 /* TCP Server port */

//ffmpeg stream
void* ffmpeg_stream_thread(void* data)
{
    system("ffmpeg -f v4l2 -s 160x120 -i /dev/video0 -f rtp rtp://192.168.1.159:7000");

    pthread_exit(NULL);
}

void ffmpeg_stream_thread_create(pthread_t ff_stream_t)
{
    pthread_create(&ff_stream_t, 0, ffmpeg_stream_thread, NULL);
}

int main ( int argc, char* argv[] ) {
    int sockfd, newsockfd, clilen;
    struct sockaddr_in  cli_addr;
	struct sockaddr_in  serv_addr;
    char buff[30];
   	int size;
    int val_set;

    pthread_t ff_stream_t;

    pthread_t th_ultrasonic;
    void *result_ultrasonic;

	//ffmpeg_stream_start
    ffmpeg_stream_thread_create(ff_stream_t);
	
// mknod ultrasonic
    ultra_mknod();
// mknod moter
    moter_mknod();
    //create tcp socket to get sockfd
    if ((sockfd = socket(AF_INET, SOCK_STREAM,0))<0) {
        puts( "Server: Cannot open Stream Socket.");
        exit(1);
    }

    bzero((void *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //serv_addr.sin_addr.s_addr = htonl( INADDR_ANY );
	
    //set port
    serv_addr.sin_port = htons(SERV_TCP_PORT); 
    //bind your socket with the address info

    //set reuse
    val_set = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,  &val_set, sizeof(val_set));

    if ((bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)))<0) {
        puts( "Server: Cannot bind Local Address.");
        exit(1);
    }
//set listen args      
    listen(sockfd, 5);
//call accept
    printf("Server is waiting client...\n");    

    newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen );

    if ( newsockfd < 0 ) {
        puts("Server: accept error!");
        exit(1);
    }

//ultrasonic thread
    if ( pthread_create(&th_ultrasonic, NULL, &ultra_func, newsockfd) != 0) {
        puts("ultrasonic pthread_create() error!");    
        exit(1);
    }

    clilen = sizeof( cli_addr );

    printf("Client Connected...\n");  

	while(1) {

		if ((size = read(newsockfd, buff, 20)) <= 0 ) {
			puts( "Server: readn error!");
			exit(1);
		}

        switch(buff[0])
        {
            case '1':
                printf("front from server\n");
		moter_func(1);
                break;
            case '2':
                printf("back from server\n");
		moter_func(2);
                break;
            case '3':
                printf("right from server\n");
		moter_func(5);
                break;
            case '4':
                printf("left from server\n");
		moter_func(6);
                break;
            case '5':
                printf("stop from server\n");
		moter_func(7);
                break;
            default:
                break;
        }
	}

    close( newsockfd );
    close( sockfd );

    return 0;
}
