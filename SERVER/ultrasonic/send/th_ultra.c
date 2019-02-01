#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

#include "th_ultra.h"

void *ultra_fun(void *data)
{
  int newsockfd = *(int *)data; 
  int size;
  char buff[30];
  int fd;
  fd = open("/dev/ultrasonic",O_RDWR);

  if(fd<0) {
    perror("/dev/ultrasonic error");

    exit(-1);
  } else {
    printf("ultrasonic has been detected ...\n");
  }
  
  while(1)
  {
    memset(buff, 0x00, 20);
    read(fd,buff,1);

    if(buff[0]==ULTRA_SIG)
    {
      if ((size = write(newsockfd, buff, strlen(buff))) <= 0) {
        puts( "Server: write error!");
      }      
    }

  }

  return 0;
}
