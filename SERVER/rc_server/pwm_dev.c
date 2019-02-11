/*
*	Filename : pwm.c
*	Title : Pwm Device
*	Desc:
*
*/

#include <linux/module.h>	//Needed by all modules
#include <linux/kernel.h>	
#include <linux/init.h>		//Needed for the macros

#include <linux/fs.h>		//Needed for cdev			inode structure
#include <linux/cdev.h>		//Needed for register cdev

#include <linux/platform_device.h>
#include <linux/major.h>	//Needed for open rlease

#include <asm/uaccess.h>	//Needed for write,read
#include <linux/ioctl.h>	//Needed for ioctl cmd
#include <linux/pwm.h>

#include <linux/gpio.h>

#include "pwm_dev.h"
#include "moter_app.h"
#include <plat/gpio-cfg.h>
#include <plat/s3c2416.h>
#include <linux/slab.h>

#include <linux/vmalloc.h>


MODULE_LICENSE("GPL");


/*TODO : Initialization*/
static int pwm_major=DEV_PWM_MAJOR, pwm_minor=0;
static int result;
static dev_t pwm_dev;


/*allocate records in file_operations*/
struct file_operations pwm_fops; //file operations
static struct cdev pwm_cdev;


/*pwm*/

static struct pwm_pulse lpwm_pulse;
static struct pwm_pulse rpwm_pulse;

static struct pwm_device *left_pwm;//
static struct pwm_device *right_pwm;// 
    

/*TODO :Define Prototype of fuctions*/
static int pwm_init(void);
static void pwm_exit(void);

static int pwm_register_cdev(void);

static int pwm_open(struct inode *inode,struct file *filp);
static int pwm_release(struct inode *inode, struct file *filp);

static int pwm_write(struct file *filp, const char *buf, size_t count,loff_t *f_pos);
static int pwm_read(struct file *filp, char *buf, size_t count,loff_t *f_pos);


/*TODO :Implementation of functions*/

static int pwm_open(struct inode *inode,struct file *filp)
{
	printk("Device has been opened...\n");
	/* H/W Initalization */
	left_pwm = pwm_request(2,"left_pwm");
	right_pwm = pwm_request(3,"right_pwm");

	
if( (NULL==left_pwm) || (NULL==right_pwm) )
	{
		printk("fail!!!!");
		return -1;
	}	
	printk("pwm_request COMPLETED...\n");

	pwm_config(left_pwm, 0, 0);
	pwm_config(right_pwm, 0, 0);
	
	
	return 0;
}

static int pwm_release(struct inode *inode, struct file *filp)
{
	printk("Device has been closed...\n");
	
	pwm_free(left_pwm);
	pwm_free(right_pwm);
	
	return 0;
}


static int pwm_write(struct file *filp, const char *buf, size_t count,loff_t *f_pos)
{
//	char data[11];
//	printk("\ndata >>>>> = %s\n",data);

	return count;
}

static int pwm_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
//	char data[20] = "this is read func...";
	/*transfer app_from_address buf*/
//	copy_to_user(buf,data,count);

	return 0;
}

static int pwm_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
{
//	printk("pwm_ioctl : %d\n", cmd);
	switch(cmd){
		case FRONT :{
			gpio_set_value(S3C2410_GPG(4), 0);	// EN-HIGH
			gpio_set_value(S3C2410_GPG(7), 0);	// EN-
			gpio_set_value(S3C2410_GPG(5), 1);	// state
			gpio_set_value(S3C2410_GPG(6), 0);
			pwm_enable(left_pwm);
			pwm_enable(right_pwm);
			//printk("dev.strit X1> LR pwm should be  enabled \n");
		}break;
		
		case BACK :{
			gpio_set_value(S3C2410_GPG(4), 0);	// EN-motor-on
			gpio_set_value(S3C2410_GPG(7), 0);	// EN_HIGH
			gpio_set_value(S3C2410_GPG(5), 0);
			gpio_set_value(S3C2410_GPG(6), 1);
			pwm_enable(left_pwm);
			pwm_enable(right_pwm);
			//printk("dev.strit X2> LR pwm should be  enabled \n");
		}break;
		
		case LEFT :{
			gpio_set_value(S3C2410_GPG(4), 1);	// EN-motor-on
			gpio_set_value(S3C2410_GPG(7), 0);	// 
			//gpio_set_value(S3C2410_GPG(5), 1);
			//gpio_set_value(S3C2410_GPG(6), 0);
			pwm_enable(left_pwm);
			pwm_enable(right_pwm);
			//printk("dev.strit X3> LR pwm should be  enabled \n");
			//printk("kcmd : %d\n", cmd);
		}break;

		case RIGHT :{
			gpio_set_value(S3C2410_GPG(4), 0);	// EN-motor-on
			gpio_set_value(S3C2410_GPG(7), 1);	// 
			//gpio_set_value(S3C2410_GPG(5), 0);
			//gpio_set_value(S3C2410_GPG(6), 1);
			pwm_enable(left_pwm);
			pwm_enable(right_pwm);
			//printk("dev.strit X4> LR pwm should be  enabled \n");
		}break;

		case STOP :{
			gpio_set_value(S3C2410_GPG(4), 0);	// EN-motor-on
			gpio_set_value(S3C2410_GPG(7), 0);	// 
			gpio_set_value(S3C2410_GPG(5), 0);
			gpio_set_value(S3C2410_GPG(6), 0);
			pwm_enable(left_pwm);
			pwm_enable(right_pwm);
			//printk("dev.strit X5> LR pwm should be  enabled \n");
		}break;
	
		case LEFTPWM :{
			copy_from_user((void *)&lpwm_pulse, (const void *)arg, sizeof( struct pwm_pulse ));
			pwm_config(left_pwm, lpwm_pulse.width, lpwm_pulse.period);
			//printk("dev.ioctl L> Duty: %lu, Period: %lu\n", lpwm_pulse.width, lpwm_pulse.period );
		//}break;
		//case RIGHTPWM :{
			copy_from_user((void *)&rpwm_pulse, (const void *)arg, sizeof( struct pwm_pulse ));
			pwm_config(right_pwm, rpwm_pulse.width, rpwm_pulse.period);
			//printk("dev.ioctl R> Duty: %lu, Period: %lu\n", rpwm_pulse.width, rpwm_pulse.period );
		}break;
		default :
			return 0;
	}
	return 0;
}


struct file_operations pwm_fops = {
	.open = pwm_open,
	.release = pwm_release,
	.write = pwm_write,
	.read = pwm_read,
	.unlocked_ioctl = pwm_ioctl,
};

static int pwm_init(void)
{
	printk("PWM_DEV_MODULE IS UP\n");
	/*register pin*/ 	
	s3c_gpio_cfgpin(S3C2410_GPB(2), S3C_GPIO_SFN(2));
	s3c_gpio_setpull(S3C2410_GPB(2), S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(S3C2410_GPB(3), S3C_GPIO_SFN(3));
	s3c_gpio_setpull(S3C2410_GPB(3), S3C_GPIO_PULL_UP);
	/*timer 2,3*/
	s3c_gpio_cfgpin(S3C2410_GPB(2), S3C_GPIO_SFN(2));
	s3c_gpio_setpull(S3C2410_GPB(2), S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(S3C2410_GPB(3), S3C_GPIO_SFN(3));
	s3c_gpio_setpull(S3C2410_GPB(3), S3C_GPIO_PULL_UP);
	/*gpio set*/
	gpio_set_value(S3C2410_GPG(4), 0); //EN-motor on:0 off:1
	gpio_set_value(S3C2410_GPG(7), 0); //
	gpio_set_value(S3C2410_GPG(5), 0); //	
	gpio_set_value(S3C2410_GPG(6), 0); //
	
	/*register char_dev*/
	if((result= pwm_register_cdev())<0)
	{
		return result;
	}

	return 0;
}

static void pwm_exit(void)
{
	printk("PWM_DEV_EXIT\n");
	
	cdev_del(&pwm_cdev);	//delete cdev
	unregister_chrdev_region(pwm_dev,1);
}

static int pwm_register_cdev(void)
{
	int error;

	/*allocation device number*/
	if(pwm_major) {
		pwm_dev = MKDEV(pwm_major,pwm_minor);	//num setting
		error = register_chrdev_region(pwm_dev, 1, "pwm");		
	} else {
		error = alloc_chrdev_region(&pwm_dev,pwm_minor, 1, "pwm");
		pwm_major = MAJOR(pwm_dev);
	}

	if(error<0){
		printk(KERN_WARNING "pwm_num Register Error %d\n", error);
		return error;
	}

	printk("major number=%d\n",pwm_major);

	/*register chrdev*/
	cdev_init(&pwm_cdev, &pwm_fops);
	pwm_cdev.owner = THIS_MODULE;
	pwm_cdev.ops = &pwm_fops;
	error =cdev_add(&pwm_cdev, pwm_dev, 1);

	if(error)
		printk(KERN_NOTICE "pwm_chrdev Register Error %d\n",error);
	
	return 0;
}


module_init(pwm_init);
module_exit(pwm_exit);
