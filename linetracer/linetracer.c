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

#include "linetracer.h"


static int linetracer_major=DEV_LINE_MAJOR, linetracer_minor=0;
static int result;
static dev_t linetracer_dev;
static int lineValue[3] = {0,};
static struct cdev linetracer_cdev;

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
		
		/* create pulse to linetracer */
		lineValue[0] = gpio_get_value(S3C2410_GPG(1));
		lineValue[1] = gpio_get_value(S3C2410_GPG(2));
		lineValue[2] = gpio_get_value(S3C2410_GPG(3));

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

static int linetracer_open(struct inode *inode, struct file *filp)
{
	int ret;
	
	kerneltimer_init();
  	
	/* linetracer sensor init */
	gpio_request(S3C2410_GPG(1), "LEFT");
	gpio_request(S3C2410_GPG(2), "MIDL");
	gpio_request(S3C2410_GPG(3), "RIGH");
	
	s3c_gpio_cfgpin(S3C2410_GPG(1), S3C_GPIO_SFN(0));
	s3c_gpio_cfgpin(S3C2410_GPG(2), S3C_GPIO_SFN(0));
	s3c_gpio_cfgpin(S3C2410_GPG(3), S3C_GPIO_SFN(0));
	
	kerneltimer_registertimer( ptrmng, TIME_STEP );
	printk("Device has been opened...\n");
	
	return 0;
}

static int linetracer_release(struct inode *inode, struct file *filp)
{ 
	printk("Device has been closed .. \n");
	
	return 0;
}

static int linetracer_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
	char data[11];
	// copy_from_user(data,buf,count);
	// printk("data>>>>=%s\n",data);
	
	return count;
}

static int linetracer_read(struct file *filp, int *buf, size_t count,loff_t *f_pos)
{
	int loop;
	int i;
	
	
	copy_to_user(buf, lineValue, 12);
	

	for(i = 0; i < 3; i++)
		lineValue[i] = 0;
	return 0;
}

struct file_operations linetracer_fops = {
	.open = linetracer_open,
	.release = linetracer_release,
	.write = linetracer_write,
	.read = linetracer_read
};

static int linetracer_init(void)
{
  
	printk("linetracer MODULE is up ...\n");
	if((result=linetracer_register_cdev())<0)
	{
		return result;
	}
	
	return 0;
}

static void linetracer_exit(void)
{ 
	printk("the module is down...\n");
	
	kerneltimer_exit();
	
	/* linetracer sensor free */
	gpio_free(S3C2410_GPG(1));
	gpio_free(S3C2410_GPG(2));
	gpio_free(S3C2410_GPG(3));

	cdev_del(&linetracer_cdev);
	unregister_chrdev_region(linetracer_dev, 1);

}


static int linetracer_register_cdev(void)
{
	int error;
	
	if(linetracer_major)
	{
		linetracer_dev=MKDEV(linetracer_major, linetracer_minor);
		error = register_chrdev_region(linetracer_dev, 1, "linetracer");
	}
	else
	{
		error = alloc_chrdev_region(&linetracer_dev, linetracer_minor,1,"linetracer");
		linetracer_major = MAJOR(linetracer_dev);
	}
	
	if(error <0)
	{
		printk(KERN_WARNING "linetracer: cant get major %d \n",linetracer_major);
		
		return result;
	}
	printk("DEV_LINE_MAJOR: %d linetracer_major: %d\n", linetracer_major, linetracer_minor);
	
	cdev_init(&linetracer_cdev, &linetracer_fops);
	linetracer_cdev.owner = THIS_MODULE;
	linetracer_cdev.ops = &linetracer_fops;
	error = cdev_add(&linetracer_cdev, linetracer_dev, 1);
	
	if(error)
		printk(KERN_NOTICE "linetracer Register Error %d \n",error);
	
	return 0;
}

module_init(linetracer_init);
module_exit(linetracer_exit);

MODULE_LICENSE("Dual BSD/GPL");

