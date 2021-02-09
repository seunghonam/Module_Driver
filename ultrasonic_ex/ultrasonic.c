#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/gpio-fns.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/uaccess.h>
#include <asm/types.h>
#include <linux/device.h>

#include "ultrasonic.h"


static int ultrasonic_major=DEV_ULTRA_MAJOR, ultrasonic_minor=0;
static int result;
static dev_t ultrasonic_dev;
static int distance_cm[2] = {0,};
static int ultra_cnt = 0;
static int ultra_cnt2 = 0;
static struct cdev ultrasonic_cdev;

static irqreturn_t ultra_echo_rising(int irq, void *dev_id, struct pt_regs *regs)
{

	if ( gpio_get_value(S3C2410_GPG(5) ) )  // when rising trigger occured!   ( ECHO = HIGH   <- ___| | | | | | | | ____
	{
		do_gettimeofday(&before);   // capture time of head of return signal of high.  we need TRIGGER_RISING
	}
	else
	{
		do_gettimeofday(&after);	// capture time of tail of return signal while high. we need TRIGGER_FALLING
	
		distance_cm[0] = ((after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec ) ) /58;      
		
		//printk(" distance1 cm : %d \n", distance_cm[0] );
		/*
		if ( distance_cm[0] < ULTRA_SIG )
		{
			ultra_cnt++;
		}
		else
		{
			ultra_cnt=0;
		} 
		*/
		memset(&before, 0 , sizeof(struct timeval ) );
		memset(&after, 0 , sizeof(struct timeval ) );
	}
	
	return IRQ_HANDLED;
}

static irqreturn_t ultra_echo_rising2(int irq, void *dev_id, struct pt_regs *regs)
{

	if ( gpio_get_value(S3C2410_GPG(15) ) )  // when rising trigger occured!   ( ECHO = HIGH   <- ___| | | | | | | | ____
	{
		do_gettimeofday(&before2);   // capture time of head of return signal of high.  we need TRIGGER_RISING
	}
	else
	{
		do_gettimeofday(&after2);	// capture time of tail of return signal while high. we need TRIGGER_FALLING
	
		distance_cm[1] = ((after2.tv_sec - before2.tv_sec) * 1000000 + (after2.tv_usec - before2.tv_usec ) ) /58;      
		
		//printk(" distance2 cm : %d \n", distance_cm[1] );
		/*
		if ( distance_cm[1] < ULTRA_SIG )
		{
			ultra_cnt2++;
		}
		else
		{
			ultra_cnt2=0;
		} 
		*/
		memset(&before2, 0 , sizeof(struct timeval ) );
		memset(&after2, 0 , sizeof(struct timeval ) );
	}
	
	return IRQ_HANDLED;
}

void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover )
{
	init_timer( &(pdata->timer) );
	pdata->timer.expires  = get_jiffies_64() + timeover;
	pdata->timer.data     = (unsigned long) pdata      ;
	pdata->timer.function = kerneltimer_timeover       ;
	add_timer( &(pdata->timer) );
}

void kerneltimer_timeover(unsigned long arg )
{
	KERNEL_TIMER_MANAGER *pdata = NULL;     
   
	if( arg )
	{
		pdata = ( KERNEL_TIMER_MANAGER * ) arg;
		
		/* create pulse to ultrasonic */
		gpio_set_value(S3C2410_GPG(4), 0);		// TRIGGER  _
		gpio_set_value(S3C2410_GPG(7), 0);
		udelay(20);
		gpio_set_value(S3C2410_GPG(4), 1);		// TRIGGER  ^_
		gpio_set_value(S3C2410_GPG(7), 1);
		udelay(20);
		gpio_set_value(S3C2410_GPG(4), 0);		// TRIGGER  _^_
		gpio_set_value(S3C2410_GPG(7), 0);
		
		kerneltimer_registertimer( pdata , TIME_STEP );
	}
}

int kerneltimer_init(void)
{
	ptrmng = kmalloc( sizeof( KERNEL_TIMER_MANAGER ), GFP_KERNEL );
	if( ptrmng == NULL ) return -ENOMEM;
	
	memset( ptrmng, 0, sizeof( KERNEL_TIMER_MANAGER ) );
	
	return 0;
}

void kerneltimer_exit(void)
{
	if( ptrmng != NULL ) 
	{
		del_timer( &(ptrmng->timer) );
		kfree( ptrmng );
	}    
}

static int ultrasonic_open(struct inode *inode, struct file *filp)
{
	int ret;
	
	kerneltimer_init();
  	
	/* ultrasonic sensor init */
	gpio_request(S3C2410_GPG(4),  "TRIG1");
	gpio_request(S3C2410_GPG(5),  "ECHO1");	// IRT_EINT10  <---
	gpio_request(S3C2410_GPG(7),  "TRIG2");
	gpio_request(S3C2410_GPG(15), "ECHO2");
	
	s3c_gpio_cfgpin(S3C2410_GPG(4),  S3C_GPIO_SFN(1));	// TRIGGER OUTPUT : SHOT  [ ] --->  ( oo )
	s3c_gpio_cfgpin(S3C2410_GPG(5),  S3C_GPIO_SFN(2));	// INTERRUPT INPUT: ECHO  [ ] <---  ( oo )
	s3c_gpio_cfgpin(S3C2410_GPG(7),  S3C_GPIO_SFN(1));	// TRIGGER OUTPUT : SHOT  [ ] --->  ( oo )
	s3c_gpio_cfgpin(S3C2410_GPG(15), S3C_GPIO_SFN(2));	// INTERRUPT INPUT: ECHO  [ ] <---  ( oo )
	
	if(request_irq(IRQ_EINT13,(void *)ultra_echo_rising, IRQF_DISABLED|IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "echoint2",NULL))
	{
		ret=-ENOENT;
		printk("failed to request external interrupt microsonar. %d\n",ret);
		
		return ret;
	}
	if(request_irq(IRQ_EINT23,(void *)ultra_echo_rising2, IRQF_DISABLED|IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "echoint3", NULL))
	{
		ret=-ENOENT;
		printk("failed to request external interrupt microsonar. %d\n",ret);
		
		return ret;
	}
	
	kerneltimer_registertimer( ptrmng, TIME_STEP );
	printk("Device has been opened...\n");
	
	return 0;
}

static int ultrasonic_release(struct inode *inode, struct file *filp)
{ 
	free_irq(IRQ_EINT13, NULL);
	free_irq(IRQ_EINT23, NULL);
	printk("device has been closed .. \n");
	
	return 0;
}

static int ultrasonic_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
	char data[11];
	//  copy_from_user(data,buf,count);
	printk("data>>>>=%s\n",data);
	
	return count;
}

static int ultrasonic_read(struct file *filp, char *buf, size_t count,loff_t *f_pos)
{
	int i = 0;
	int loop;
	char distance_flag[2];
/*
	if ( ultra_cnt > 5 )
	{
		distance_flag[0] = ULTRA_SIG;
	
	}
	else
	{
		distance_flag[0] = 0;
	}

	if ( ultra_cnt2 > 5 )
	{
		distance_flag[1] = ULTRA_SIG;
	
	}
	else
	{
		distance_flag[1] = 0;
	}

	for(loop=0; loop<count;loop++)
	{
		for(i = 0; i < 2; ++i)
			put_user(distance_flag[i], (int *)buf[i]);
	}
*/
	
	distance_flag[0] = ((distance_cm[0] < 15) ? '1' : '0');
	distance_flag[1] = ((distance_cm[1] < 15) ? '1' : '0');	

	copy_to_user( buf, distance_flag, 2 );
	
	return 0;
}

struct file_operations ultrasonic_fops = {
	.open = ultrasonic_open,
	.release = ultrasonic_release,
	.write = ultrasonic_write,
	.read = ultrasonic_read
};

static int ultrasonic_init(void)
{
  
	printk("ultrasonic MODULE is up ...\n");
	if((result=ultrasonic_register_cdev())<0)
	{
		return result;
	}
	
	return 0;
}

static void ultrasonic_exit(void)
{ 
	printk("the module is down...\n");
	
	kerneltimer_exit();
	
	/* ultrasonic sensor free */
	gpio_free(S3C2410_GPG(4));
	gpio_free(S3C2410_GPG(5));
	gpio_free(S3C2410_GPG(7));
	gpio_free(S3C2410_GPG(15));
	cdev_del(&ultrasonic_cdev);
	unregister_chrdev_region(ultrasonic_dev,1);

}


static int ultrasonic_register_cdev(void)
{
	int error;
	
	if(ultrasonic_major)
	{
		ultrasonic_dev=MKDEV(ultrasonic_major, ultrasonic_minor);
		error = register_chrdev_region(ultrasonic_dev, 1, "ultrasonic");
	}
	else
	{
		error = alloc_chrdev_region(&ultrasonic_dev, ultrasonic_minor,1,"ultrasonic");
		ultrasonic_major = MAJOR(ultrasonic_dev);
	}
	
	if(error <0)
	{
		printk(KERN_WARNING "ultrasonic: cant get major %d \n",ultrasonic_major);
		
		return result;
	}
	printk("DEV_ULTRA_MAJOR: %d ultrasonic_minor: %d\n", DEV_ULTRA_MAJOR, ultrasonic_minor);
	printk("major number = %d \n",ultrasonic_major);
	
	cdev_init(&ultrasonic_cdev, &ultrasonic_fops);
	ultrasonic_cdev.owner = THIS_MODULE;
	ultrasonic_cdev.ops = &ultrasonic_fops;
	error = cdev_add(&ultrasonic_cdev, ultrasonic_dev, 1);
	
	if(error)
		printk(KERN_NOTICE "ultrasonic Register Error %d \n",error);
	
	return 0;
}

module_init(ultrasonic_init);
module_exit(ultrasonic_exit);

MODULE_LICENSE("Dual BSD/GPL");

