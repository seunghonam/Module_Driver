#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#define   DRV_NAME            "ultrasonic"
#define   ULTRA_SIG           15
#define   DEV_ULTRA_MAJOR 	103
#define   TIME_STEP           (HZ/100)

typedef struct
{
    struct timer_list  timer;      
} __attribute__ ((packed)) KERNEL_TIMER_MANAGER;
static KERNEL_TIMER_MANAGER *ptrmng = NULL;

struct ultra_detection
{
	int irq;
	int pin;
	int pin_setting;
	char *name;
	int last_state;
};

struct timeval before = {0,}, after = {0,}, before2 = {0,}, after2 = {0,};

int  kerneltimer_init(void);
void kerneltimer_exit(void);
void kerneltimer_timeover(unsigned long arg );
void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover );
static int ultrasonic_register_cdev(void);
static irqreturn_t ultra_echo_rising(int irq, void *dev_id, struct pt_regs *regs);
static irqreturn_t ultra_echo_rising2(int irq, void *dev_id, struct pt_regs *regs);
#endif //__ULTRASONIC_H__
