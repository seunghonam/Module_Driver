#ifndef __LINETRACER_H__
#define __LINETRACER_H__

#define   DRV_NAME            "linetracer"
#define   LINE_SIG           1
#define   DEV_LINE_MAJOR 	104
#define   TIME_STEP           (HZ/100) // 10ms

typedef struct
{
    struct timer_list  timer;      
} __attribute__ ((packed)) KERNEL_TIMER_MANAGER;
static KERNEL_TIMER_MANAGER *ptrmng = NULL;

struct line_detection
{
	int irq;
	int pin;
	int pin_setting;
	char *name;
	int last_state;
};

struct timeval before = {0,}, after = {0,};

int kerneltimer_init(void);
void kerneltimer_exit(void);
void kerneltimer_timeover(unsigned long arg );
void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover );
static int linetracer_register_cdev(void);

#endif //__LINETRACER_H__
