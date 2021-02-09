#ifndef __TH_ULTRA_H__
#define __TH_ULTRA_H__

#define	ULTRA_SIG           15
#define DEV_ULTRA_NAME  	"ultrasonic"
#define	DEV_ULTRA_MAJOR 	103
#define DOUBLE 1

void ultra_mknod(void);
void *ultra_func(void *data);

#endif //__TH_ULTRA_H__