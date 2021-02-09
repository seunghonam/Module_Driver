#ifndef __DHT_H__
#define __DHT_H__

#define   DRV_NAME            "dht11"
#define   LINE_SIG           1
#define   DEV_LINE_MAJOR 	105
#define   TIME_STEP           (HZ/100) // 1000ms

typedef struct
{
    struct timer_list  timer;      
} __attribute__ ((packed)) KERNEL_TIMER_MANAGER;
static KERNEL_TIMER_MANAGER *ptrmng = NULL;

struct dht_detection
{
	int irq;
	int pin;
	int pin_setting;
	char *name;
	int last_state;
};

struct timeval before = {0,}, after = {0,};

static irqreturn_t dht11_echo_rising(int irq, void *dev_id, struct pt_regs *regs);
int kerneltimer_init(void);
void kerneltimer_exit(void);
void kerneltimer_timeover(unsigned long arg );
void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover );
static int dht11_register_cdev(void);

#endif //__DHT_H__
