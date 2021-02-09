#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/major.h>
#include <linux/types.h>
#include <linux/kdev_t.h>

#include "th_ultra.h"

void ultra_mknod(void)
{
    char dev_name[1024];  

    bzero( dev_name, sizeof( dev_name ));
    sprintf( dev_name, "/dev/%s", DEV_ULTRA_NAME );
    mknod( dev_name, S_IFCHR|S_IRWXU|S_IRWXG, MKDEV( DEV_ULTRA_MAJOR, 0 ));
    printf("Make Device file(%s)\n", dev_name );

    return;
}

void *ultra_func(void *data)
{
    int newsockfd = *(int *)data; 
    int size;
    char buff[2];
    int fd;



    fd = open("/dev/ultrasonic",O_RDWR);

    if(fd<0)
	{
        perror("/dev/ultrasonic error");

        exit(-1);
    }
	else
	{
        printf("ultrasonic has been detected ...\n");
    }
    printf("ready!\n");
    
    while(1)
    {
        int debug = 0;
        memset(buff, 0x00, sizeof(buff) );
        debug = read(fd, buff, sizeof(buff));
        printf("debug number : %d\n", debug);  
        printf("front value:   %c\n", buff[0]);
        printf("back  value:   %c\n", buff[1]);
        usleep(1000000);
        /*
        if(buff[0]==ULTRA_SIG)
        {
        	printf("write!\n");
            if ((size = write(newsockfd, buff, strlen(buff))) <= 0)
			{
                puts( "Server: write error!");
            }
        }
        */
    }

    return 0;
}
