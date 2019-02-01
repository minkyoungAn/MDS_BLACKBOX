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

#define SERV_TCP_PORT   6000 /* TCP Server port */


int main ( int argc, char* argv[] ) {
    int sockfd, newsockfd, clilen;
    struct sockaddr_in  cli_addr;
	struct sockaddr_in  serv_addr;
    char buff[30];
   	int size;
    pthread_t th_ultrasonic;
    void *result_ultrasonic;

// mknod ultrasonic
    ultra_mknod();

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
                break;
            case '2':
                printf("back from server\n");
                break;
            case '3':
                printf("right from server\n");
                break;
            case '4':
                printf("left from server\n");
                break;
            case '5':
                printf("stop from server\n");
                break;
            default:
                break;
        }
	}

    close( newsockfd );
    close( sockfd );

    return 0;
}
