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

#include "dht11.h"


static int dht11_major=DEV_LINE_MAJOR, dht11_minor=0;
static int result;
static dev_t dht11_dev;
static int buf_temp_moisture;
static struct cdev dht11_cdev;
static int buf_count;

/*
static irqreturn_t dht11_echo_rising(int irq, void *dev_id, struct pt_regs *regs)
{

	if ( buf_count == 0 && gpio_get_value(S3C2410_GPG(15) ) )  // when rising trigger occured!   ( ECHO = HIGH   <- ___| | | | | | | | ____
	{
		do_gettimeofday(&before);   // capture time of head of return signal of high.  we need TRIGGER_RISING
	}
	else
	{
		do_gettimeofday(&after);	// capture time of tail of return signal while high. we need TRIGGER_FALLING
	
		distance_cm[0] = ((after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec ) ) /58;      
		
		memset(&before, 0 , sizeof(struct timeval ) );
		memset(&after, 0 , sizeof(struct timeval ) );
	}
	
	buf_count++;

	if ( buf_count == 40 ) {
		s3c_gpio_cfgpin(S3C2410_GPG(15), S3C_GPIO_SFN(2));
	}

	return IRQ_HANDLED;
}
*/

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
		

		/* create pulse to dht11 */
		gpio_set_value(S3C2410_GPG(15), 1);		// HIGH
		udelay(20);
		gpio_set_value(S3C2410_GPG(15), 0);		// LOW
		udelay(18000000);
		gpio_set_value(S3C2410_GPG(15), 1);		// PULL UP 
		udelay(30);

		s3c_gpio_cfgpin(S3C2410_GPG(15), S3C_GPIO_SFN(0)); // input change

		buf_count


		s3c_gpio_cfgpin(S3C2410_GPG(15), S3C_GPIO_SFN(1)); // output change

//		s3c_gpio_cfgpin(S3C2410_GPG(15), S3C_GPIO_SFN(2)); // EINT 23 change

/*
		if(request_irq(IRQ_EINT23,(void *)dht11_echo_rising, IRQF_DISABLED|IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "echoint23", NULL))
		{
			ret=-ENOENT;
			printk("failed to request external interrupt dht11. %d\n",ret);
			
			return ret;
		}
*/
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

static int dht11_open(struct inode *inode, struct file *filp)
{
	int ret;
	
	kerneltimer_init();
  	
	/* dht11 sensor init */
	gpio_request(S3C2410_GPG(15), "dht11");
	
	s3c_gpio_cfgpin(S3C2410_GPG(15), S3C_GPIO_SFN(1)); // PIN OUTPUT ¼³Á¤

	kerneltimer_registertimer( ptrmng, TIME_STEP );
	printk("Device has been opened...\n");
	
	return 0;
}

static int dht11_release(struct inode *inode, struct file *filp)
{ 
	printk("Device has been closed .. \n");
	
	return 0;
}

static int dht11_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
	char data[11];
	// copy_from_user(data,buf,count);
	// printk("data>>>>=%s\n",data);
	
	return count;
}

static int dht11_read(struct file *filp, int *buf, size_t count,loff_t *f_pos)
{
	int loop;
	int i;
	
	
	copy_to_user(buf, lineValue, 12);
	

	for(i = 0; i < 3; i++)
		lineValue[i] = 0;
	return 0;
}

struct file_operations dht11_fops = {
	.open = dht11_open,
	.release = dht11_release,
	.write = dht11_write,
	.read = dht11_read
};

static int dht11_init(void)
{
  
	printk("dht11 MODULE is up ...\n");
	if((result=dht11_register_cdev())<0)
	{
		return result;
	}
	
	return 0;
}

static void dht11_exit(void)
{ 
	printk("the module is down...\n");
	
	kerneltimer_exit();
	
	/* dht11 sensor free */
	gpio_free(S3C2410_GPG(15));

	cdev_del(&dht11_cdev);
	unregister_chrdev_region(dht11_dev, 1);

}


static int dht11_register_cdev(void)
{
	int error;
	
	if(dht11_major)
	{
		dht11_dev=MKDEV(dht11_major, dht11_minor);
		error = register_chrdev_region(dht11_dev, 1, "dht11");
	}
	else
	{
		error = alloc_chrdev_region(&dht11_dev, dht11_minor,1,"dht11");
		dht11_major = MAJOR(dht11_dev);
	}
	
	if(error <0)
	{
		printk(KERN_WARNING "dht11: cant get major %d \n",dht11_major);
		
		return result;
	}
	printk("DEV_LINE_MAJOR: %d dht11_minor: %d\n", dht11_major, dht11_minor);
	
	cdev_init(&dht11_cdev, &dht11_fops);
	dht11_cdev.owner = THIS_MODULE;
	dht11_cdev.ops = &dht11_fops;
	error = cdev_add(&dht11_cdev, dht11_dev, 1);
	
	if(error)
		printk(KERN_NOTICE "dht11 Register Error %d \n",error);
	
	return 0;
}

module_init(dht11_init);
module_exit(dht11_exit);

MODULE_LICENSE("Dual BSD/GPL");

