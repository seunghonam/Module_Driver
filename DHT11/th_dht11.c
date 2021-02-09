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

#include "th_dht11.h"

void dht11_mknod(void)
{
    char dev_name[1024];  

    bzero( dev_name, sizeof( dev_name ));
    sprintf( dev_name, "/dev/%s", DEV_LINE_NAME );
    mknod( dev_name, S_IFCHR|S_IRWXU|S_IRWXG, MKDEV( DEV_LINE_MAJOR, 0 ));
    printf("Make Device file(%s)\n", dev_name );

    return;
}

void *dht11_func(void *data)
{
    int newsockfd = *(int *)data; 
    int size;
    int buff[30];
    int fd;
    int i;
    char buf_name[3] = {'l', 'm', 'r'};
    fd = open("/dev/dht11",O_RDWR);

    if(fd<0)
	{
        perror("/dev/dht11 error");

        exit(-1);
    }
	else
	{
        printf("dht11 has been detected ...\n");
    }

    while(1)
    {
        printf("ready!\n");
        memset(buff, 0x00, 20);
        read(fd, buff, 1);
        
        for(i=0; i<3; i++)
            printf("value : %s\n",  buf_name[i] ,(buff[i] ? "line" : "else")); 
        
		usleep(1000000);
    }

    return 0;
}
