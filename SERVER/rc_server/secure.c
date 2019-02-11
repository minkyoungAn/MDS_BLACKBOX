#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include "led.h"
#include "pwm_dev.h"

/*****license check*****/
MODULE_LICENSE("GPL");


/*****module variable*****/
static int secure_major = DEV_SK_MAJOR, secure_minor = 0;//after allocate major number
static int result;
static dev_t secure_dev;


/*****file_operation's records allocate*****/
static struct cdev secure_cdev;


static int secure_register_cdev(void);
static int secure_open(struct inode *inode, struct file *filp);
static int secure_release(struct inode *inode, struct file *filp);
static int secure_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int secure_read(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int secure_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);




static int secure_open(struct inode *inode, struct file *filp)
{
	printk("device has been opened...\n");

	/*****GPG1,3 is output(becouse of SFN1)*****/
	s3c_gpio_cfgpin(S3C2410_GPG(11), S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(12), S3C_GPIO_SFN(1));

	gpio_set_value(S3C2410_GPG(11), 0);
	gpio_set_value(S3C2410_GPG(12), 0);

	return 0;
}






static int secure_release(struct inode *inode, struct file *filp)
{
	printk("Device has been closed..\n");

	return 0;
}







static int secure_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	char data[11];
	copy_from_user(data, buf, count);

	return count;
}








static int secure_read(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	char data[20] = "this is read func";
	copy_to_user(buf, data, count);

	return 0;
}






static int secure_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{ 
	switch(cmd) 
	{
		case LEFT :
		{
			printk("led on : %d\n", cmd);
			gpio_set_value(S3C2410_GPG(12),1);
			mdelay(500);
			printk("led off : %d\n", cmd);
			gpio_set_value(S3C2410_GPG(12),0);
			mdelay(500);
		}break;

		case RIGHT :
		{
			printk("led on : %d\n", cmd);
			gpio_set_value(S3C2410_GPG(11),1);
			mdelay(500);
			printk("led off : %d\n", cmd);
			gpio_set_value(S3C2410_GPG(11),0);
			mdelay(500);
		}break;
	}
}






static struct file_operations secure_fops = 
{
	.open           = secure_open,
	.release        = secure_release,
	.write          = secure_write,
	.read           = secure_read,
	.unlocked_ioctl = secure_ioctl,
};


/*****init module*****/
static int __init secure_init(void)
{
	printk("Secure module is up... \n");

	if((result = secure_register_cdev()) < 0)
		return result;
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

