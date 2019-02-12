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

#include "led.h"
#include "pwm_dev.h"

/*****license check*****/
MODULE_LICENSE("GPL");


/*****module variable*****/
static int led_major = DEV_LED_MAJOR, led_minor = 0;//after allocate major number
static int result;
static dev_t led_dev;

static int led_flag = 0;

/*****file_operation's records allocate*****/
static struct cdev led_cdev;

static int led_register_cdev(void);
static int led_open(struct inode *inode, struct file *filp);
static int led_release(struct inode *inode, struct file *filp);
static int led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

int led_high_low = 0;

void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover )
{
	init_timer( &(pdata->timer) );
	pdata->timer.expires  = (get_jiffies_64() + timeover);
	pdata->timer.data     = (unsigned long) pdata      ;
	pdata->timer.function = kerneltimer_timeover       ;
	add_timer( &(pdata->timer) );
}

void kerneltimer_timeover(unsigned long arg )
{
	KERNEL_TIMER_MANAGER *pdata = NULL;     
   	if(led_flag == 5)
   	{
   		if(led_high_low == 0)
   		{
   			gpio_set_value(S3C2410_GPG(12), 1);	
   			led_high_low = 1;
		}
		else 
		{
			gpio_set_value(S3C2410_GPG(12), 0);	
			led_high_low = 0;
		}
   	}
   	else if(led_flag == 6)
   	{
  	 	if(led_high_low == 0)
   		{
   			gpio_set_value(S3C2410_GPG(11), 1);	
   			led_high_low = 1;
		}
		else 
		{
			gpio_set_value(S3C2410_GPG(11), 0);	
			led_high_low = 0;
		}
   	}
   	
	if( arg )
	{
		pdata = ( KERNEL_TIMER_MANAGER * ) arg;
		/* create pulse to led */	
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


static int led_open(struct inode *inode, struct file *filp)
{
	printk("led device has been opened...\n");
	
	return 0;
}




static int led_release(struct inode *inode, struct file *filp)
{
	printk("Device has been closed..\n");

	return 0;
}




static int led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{ 
	switch(cmd) 
	{
		case LEFT :
			led_flag=5;
			gpio_set_value(S3C2410_GPG(11), 0);	
			break;

		case RIGHT :
			led_flag=6;
			gpio_set_value(S3C2410_GPG(12), 0);	
			break;
		
		default :
			led_flag=1;
			gpio_set_value(S3C2410_GPG(11), 0);	
			gpio_set_value(S3C2410_GPG(12), 0);	
			break;
	}
	return 0;
}






static struct file_operations led_fops = 
{
	.open           = led_open,
	.release        = led_release,
	.unlocked_ioctl = led_ioctl
};


/*****init module*****/
static int __init led_init(void)
{
	printk("led module is up... \n");
	
	kerneltimer_init();
	kerneltimer_registertimer( ptrmng , TIME_STEP );	
	
	/*****GPG1,3 is output(becouse of SFN1)*****/
	s3c_gpio_cfgpin(S3C2410_GPG(11), S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(12), S3C_GPIO_SFN(1));

	gpio_set_value(S3C2410_GPG(11), 0);
	gpio_set_value(S3C2410_GPG(12), 0);
	
	if((result = led_register_cdev()) < 0)
		return result;
	return 0;
}





/*****exit module*****/
static void __exit led_exit(void)
{
	printk("The led module is down...\n");
	cdev_del(&led_cdev);
	unregister_chrdev_region(led_dev, 1);
}






static int led_register_cdev(void)
{
	int error;

	if(led_major)
	{
		led_dev = MKDEV(led_major, led_minor);
		error = register_chrdev_region(led_dev,1,"led");
	}
	else
	{
		error = alloc_chrdev_region(&led_dev,led_minor,1,"led");
		led_major = MAJOR(led_dev);
	}

	if(error < 0)
	{
		printk(KERN_WARNING "led: can't get major %d\n", led_major);
		return result;
	}

	printk("major number=%d\n", led_major);


	/* register chrdev */
	cdev_init(&led_cdev, &led_fops);
	led_cdev.owner = THIS_MODULE;
	led_cdev.ops = &led_fops;
	error = cdev_add(&led_cdev, led_dev, 1);


	if(error)
		printk(KERN_NOTICE "led Register Error %d\n", error);

	return 0;

}







module_init(led_init);
module_exit(led_exit);

