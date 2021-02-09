#ifndef __TH_LINE_H__
#define __TH_LINE_H__

#define	LINE_SIG           6
#define DEV_LINE_NAME  	"linetracer"
#define	DEV_LINE_MAJOR 	104

void linetracer_mknod(void);
void *linetracer_func(void *data);

#endif //__TH_LINE_H__