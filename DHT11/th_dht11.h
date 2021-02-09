#ifndef __TH_DHT_H__
#define __TH_DHT_H__

#define	LINE_SIG           6
#define DEV_LINE_NAME  	"dht11"
#define	DEV_LINE_MAJOR 	105

void dht11_mknod(void);
void *dht11_func(void *data);

#endif //__TH_DHT_H__