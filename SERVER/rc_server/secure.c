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

#include "secure.h"
#include "pwm_dev.h"

/*****license check*****/
MODULE_LICENSE("GPL");


/*****module variable*****/
static int secure_major = DEV_SK_MAJOR, secure_minor = 0;//after allocate major number
static int result;
static dev_t secure_dev;

static int led_flag = 0;

/*****file_operation's records allocate*****/
static struct cdev secure_cdev;

static int secure_register_cdev(void);
static int secure_open(struct inode *inode, struct file *filp);
static int secure_release(struct inode *inode, struct file *filp);
static int secure_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);


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
   	if(led_flag == 5)
   	{
   		gpio_set_value(S3C2410_GPG(12), 0);
		mdelay(500);
		gpio_set_value(S3C2410_GPG(12), 1);
		mdelay(500);
		gpio_set_value(S3C2410_GPG(12), 0);
   	}
   	else if(led_flag == 6)
   	{
   		gpio_set_value(S3C2410_GPG(11), 0);
		mdelay(500);
		gpio_set_value(S3C2410_GPG(11), 1);
		mdelay(500);
		gpio_set_value(S3C2410_GPG(11), 0);
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


static int secure_open(struct inode *inode, struct file *filp)
{
	printk("device has been opened...\n");
	kerneltimer_init();
	kerneltimer_registertimer( ptrmng , TIME_STEP );
	return 0;
}




static int secure_release(struct inode *inode, struct file *filp)
{
	printk("Device has been closed..\n");

	return 0;
}




static int secure_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{ 
	switch(cmd) 
	{
		case LEFT :
		{
			led_flag=5;
		}break;

		case RIGHT :
		{
			led_flag=6;
		}break;
	}
	return 0;
}






static struct file_operations secure_fops = 
{
	.open           = secure_open,
	.release        = secure_release,
	.unlocked_ioctl = secure_ioctl
};


/*****init module*****/
static int __init secure_init(void)
{
	printk("Secure module is up... \n");
		
	/*****GPG1,3 is output(becouse of SFN1)*****/
	s3c_gpio_cfgpin(S3C2410_GPG(11), S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(12), S3C_GPIO_SFN(1));

	gpio_set_value(S3C2410_GPG(11), 0);
	gpio_set_value(S3C2410_GPG(12), 0);
	
	if((result = secure_register_cdev()) < 0)
		return result;
	return 0;
}





/*****exit module*****/
static void __exit secure_exit(void)
{
	printk("The Secure module is down...\n");
	cdev_del(&secure_cdev);
	unregister_chrdev_region(secure_dev, 1);
}






static int secure_register_cdev(void)
{
	int error;

	if(secure_major)
	{
		secure_dev = MKDEV(secure_major, secure_minor);
		error = register_chrdev_region(secure_dev,1,"secure");
	}
	else
	{
		error = alloc_chrdev_region(&secure_dev,secure_minor,1,"secure");
		secure_major = MAJOR(secure_dev);
	}

	if(error < 0)
	{
		printk(KERN_WARNING "secure: can't get major %d\n", secure_major);
		return result;
	}

	printk("major number=%d\n", secure_major);


	/* register chrdev */
	cdev_init(&secure_cdev, &secure_fops);
	secure_cdev.owner = THIS_MODULE;
	secure_cdev.ops = &secure_fops;
	error = cdev_add(&secure_cdev, secure_dev, 1);


	if(error)
		printk(KERN_NOTICE "sk Register Error %d\n", error);

	return 0;

}







module_init(secure_init);
module_exit(secure_exit);

