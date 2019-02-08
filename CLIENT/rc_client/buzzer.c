#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/uaccess.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include "./buzzer.h"
#include <linux/slab.h>
#include <asm/types.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/interrupt.h>

#include <linux/errno.h>
#include <linux/kernel.h>

#include <linux/device.h>

#include <mach/gpio-fns.h>
#include <linux/time.h>
#include <linux/timer.h>

#include <plat/devs.h>
#include <plat/s3c2416.h>
#include <linux/pwm.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include "buzzer.h"

MODULE_LICENSE("GPL");

static int buzzer_major=102, buzzer_minor=0;
static int result;
static dev_t buzzer_dev;

static int buzzer_open(struct inode *inode, struct file *filp)
{

	bz_pwm = pwm_request(1, "bz_pwm"); // 1 : buzzer, 2 : TOUT2, 3 : TOUT3
	if( NULL == bz_pwm )
	{
		printk("Fail!!\n");
		return -1;
	}

	printk("Device has been opened..\n");
	pwm_duty.pulse_width = 150000;
	pwm_duty.period = 200000;
	pwm_disable(bz_pwm);
	pwm_config(bz_pwm, pwm_duty.pulse_width, pwm_duty.period);
	
	return 0;
}

static int buzzer_release(struct inode *inode, struct file *filp)
{	
	pwm_disable(bz_pwm);
	pwm_free( bz_pwm );

	printk("device has been closed .. \n");
	return 0;
}

static int buzzer_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{	

	switch(cmd){

		case BUZZER_SIG:
		{
		
//			pwm_disable(bz_pwm);

			pwm_config(bz_pwm, pwm_duty.pulse_width, pwm_duty.period);
			pwm_enable(bz_pwm);

			mdelay(100);

			break;	
		}


		default:
			return 0;

	}	

	pwm_disable(bz_pwm);

	return 0;

}

struct file_operations buzzer_fops = {
	.open = buzzer_open,	
	.release = buzzer_release,
	.unlocked_ioctl = buzzer_ioctl
};

static int buzzer_init(void)
{
	
	s3c_gpio_cfgpin(S3C2410_GPB(1), S3C_GPIO_SFN(2));
	s3c_gpio_setpull(S3C2410_GPB(1), S3C_GPIO_PULL_UP);

	printk("buzzer MODULE is up ...\n");
	if((result=buzzer_register_cdev())<0)
	{
		return result;
	}
	return 0;
}

static void buzzer_exit(void)
{	printk("the module is down...\n");

	gpio_free(S3C2410_GPB(1));

	cdev_del(&buzzer_cdev);
	unregister_chrdev_region(buzzer_dev,1);


}

static int buzzer_register_cdev(void)
{
	int error;
	if(buzzer_major){
		buzzer_dev=MKDEV(buzzer_major, buzzer_minor);
		error = register_chrdev_region(buzzer_dev, 1, "buzzer");
	}else{
		error = alloc_chrdev_region(&buzzer_dev, buzzer_minor,1,"buzzer");
		buzzer_major = MAJOR(buzzer_dev);
	}
	
	if(error <0){
		printk(KERN_WARNING "buzzer: cant get major %d \n",buzzer_major);
		return result;
	}
	
	printk("major number = %d \n",buzzer_major);

	cdev_init(&buzzer_cdev, &buzzer_fops);
	buzzer_cdev.owner = THIS_MODULE;
	buzzer_cdev.ops = &buzzer_fops;
	error = cdev_add(&buzzer_cdev, buzzer_dev, 1);

	if(error)
		printk(KERN_NOTICE "buzzer Register Error %d \n",error);
	return 0;
}

module_init(buzzer_init);
module_exit(buzzer_exit);
