#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/uaccess.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include "./sk.h"
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
//#define TIME_STEP (2*HZ/10)
#define DRV_NAME "keyint"

#define TIME_STEP 	HZ / 100



MODULE_LICENSE("GPL");


struct pwm_device {
	struct list_head     list;
	struct platform_device  *pdev;

	struct clk      *clk_div;
	struct clk      *clk;
	const char      *label;

	unsigned int         period_ns;
	unsigned int         duty_ns;

	unsigned char        tcon_base;
	unsigned char        running;
	unsigned char        use_count;
	unsigned char        pwm_id;
};

static struct pwm_device *bz_pwm;

struct pwm_duty_t pwm_duty;

static int sk_major=0, sk_minor=0;
static int result;
static char global_data=1,global_data2=3,global_data3=3;

unsigned char signal_period='b';

static int distance_cm = 0;
char distance_flag=1;
static dev_t sk_dev;

//static struct file_operations sk_fops;
static struct cdev sk_cdev;
static int sk_register_cdev(void);
static int sk_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int sk_init(void);

typedef struct
{
    struct timer_list  timer;            
	unsigned long      led;
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

struct timeval before = {0,}, after = {0,};


void kerneltimer_timeover(unsigned long arg );
void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover );
void kerneltimer_timeover(unsigned long arg );
static irqreturn_t keyinterrupt_func(int irq, void *dev_id, struct pt_regs *regs);
static irqreturn_t keyinterrupt_func2(int irq, void *dev_id, struct pt_regs *regs);
static int sk_open(struct inode *inode, struct file *filp);
static int sk_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static irqreturn_t ultra_echo_rising(int irq, void *dev_id, struct pt_regs *regs);





static irqreturn_t ultra_echo_rising(int irq, void *dev_id, struct pt_regs *regs)
{
	//int distance_cm = 0;
	if ( gpio_get_value(S3C2410_GPG(4) ) )                  // when rising trigger occured!
	{
		do_gettimeofday(&before);	
	}
	else
	{
		do_gettimeofday(&after);
		
		distance_cm = ((after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec ) ) /56;      
//		if( distance_cm < 10 )
	//	printk(" cm : %d \n", distance_cm );
		memset(&before, 0 , sizeof(struct timeval ) );
		memset(&after, 0 , sizeof(struct timeval ) );
	}

	return IRQ_HANDLED;
}

/*
static irqreturn_t ultra_echo_rising(int irq, void *dev_id, struct pt_regs *regs)
{
	if(gpio_get_value(S3C2410_GPG(4))) //when rising trigger occured!
	{
		do_gettimeofday(&before);
	}
	else
	{
		do_gettimeofday(&after);
		distance_cm = ((after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec ) ) / 56;

		printk("cm = %d \n ", distance_cm);
		memset(&before, 0, sizeof(struct timeval ));
		memset(&after, 0, sizeof(struct timeval ) );

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
	  gpio_set_value(S3C2410_GPG(5),0);
	  udelay(20);
	  gpio_set_value(S3C2410_GPG(5),1);
	  udelay(20);
	  gpio_set_value(S3C2410_GPG(5),0);

	  kerneltimer_registertimer(pdata, TIME_STEP);

 }
}


static irqreturn_t keyinterrupt_func(int irq, void *dev_id, struct pt_regs *regs)
{
	printk("\nkey pad was pressed 0 \n");
	if(global_data > 0){
		global_data-=1;
	}
	return IRQ_HANDLED;
}

static irqreturn_t keyinterrupt_func2(int irq, void *dev_id, struct pt_regs *regs)
{
	printk("\nkey pad was pressed 1 \n");
	if(global_data <2)
	{
		global_data+=1;
	}
	return IRQ_HANDLED;
}

static int sk_open(struct inode *inode, struct file *filp)
{
	int ret;
    ptrmng = kmalloc( sizeof( KERNEL_TIMER_MANAGER ), GFP_KERNEL );
    if( ptrmng == NULL ) return -ENOMEM;
    memset( ptrmng, 0, sizeof( KERNEL_TIMER_MANAGER ) );


	/* micro sonar  */

	gpio_request(S3C2410_GPG(4), "ECHO");
	gpio_request(S3C2410_GPG(5), "TRIG");

	s3c_gpio_cfgpin(S3C2410_GPG(4), S3C_GPIO_SFN(2));
	s3c_gpio_cfgpin(S3C2410_GPG(5), S3C_GPIO_SFN(1));

//	s3c_gpio_cfgpin(S3C2410_GPG(4),S3C_GPIO_SFN(1));

	/* DC motor   */
	s3c_gpio_cfgpin(S3C2410_GPG(1),S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(2),S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(3),S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(15),S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(6),S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(7),S3C_GPIO_SFN(1));

	
	/* interrupt test*/
	s3c_gpio_cfgpin(S3C2410_GPF(0),S3C_GPIO_SFN(2));
	s3c_gpio_cfgpin(S3C2410_GPF(1),S3C_GPIO_SFN(2));

	if(request_irq(IRQ_EINT0, (void *)keyinterrupt_func,IRQF_DISABLED|IRQF_TRIGGER_FALLING,DRV_NAME,NULL))
	{
		printk("failed to request external interrupt.\n");
		ret = -ENOENT;
		return ret;
	}
	
	if(request_irq(IRQ_EINT1, (void *)keyinterrupt_func2,IRQF_DISABLED|IRQF_TRIGGER_FALLING,DRV_NAME,NULL))
	{
		printk("failed to request external interrupt.\n");
		ret = -ENOENT;
		return ret;
	}

	
	if(request_irq(IRQ_EINT12,(void *)ultra_echo_rising, IRQF_DISABLED|IRQF_TRIGGER_RISING |IRQF_TRIGGER_FALLING, "echoint2",NULL)){
		ret=-ENOENT;
		printk("failed to request external interrupt microsonar. %d\n",ret);
		return ret;

	}
	
	kerneltimer_registertimer(ptrmng, TIME_STEP);
	
	bz_pwm = pwm_request(1, "bz_pwm"); // 1 : buzzer, 2 : TOUT2, 3 : TOUT3
	if( NULL == bz_pwm )
	{
		printk("Fail!!\n");
		return -1;
	}

	printk("Device has been opened..\n");
	pwm_duty.pulse_width = 150000;
	pwm_duty.period = 200000;
	pwm_config(bz_pwm, pwm_duty.pulse_width, pwm_duty.period);

	return 0;
}

static int sk_release(struct inode *inode, struct file *filp)
{	
//	free_irq(IRQ_EINT0,NULL);
//	free_irq(IRQ_EINT1,NULL);

	pwm_free( bz_pwm );

	printk("device has been closed .. \n");
	return 0;
}

static int sk_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
	char data[11];
//	copy_from_user(data,buf,count);
//	printk("data>>>>=%s\n",data);
	get_user(signal_period,buf);
	printk(" signal_period =  %c\n",signal_period);

	return count;

}

static int sk_read(struct file *filp,char *buf, size_t count,loff_t *f_pos)
{
	//char data[20] = "this is read func...";
	int loop;
	unsigned char val[10] = {global_data,global_data2,global_data3,0};

	if(distance_cm<15)
	{
		distance_flag=0;
	}else{
		distance_flag=1;
	}	

	for(loop=0; loop<count;loop++)
	{
		put_user(val[loop],(char *)&buf[0]);
		put_user(distance_flag,(char *)&buf[1]);
//		put_user(val[loop],(char *)&buf[2]);
//		put_user(val[loop],(char *)&buf[3]);

	}
	return 0;
}


struct file_operations sk_fops = {
	.open = sk_open,
	.release = sk_release,
	.write = sk_write,
	.read = sk_read,
	.unlocked_ioctl = sk_ioctl,	
};


static int sk_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{	
	if(_IOC_TYPE(cmd) != SK_MAGIC) return -EINVAL;
	if(_IOC_NR(cmd) >= SK_MAXNR) return -EINVAL;


	switch(cmd){

		case GO_STRAIGHT:
		{
			gpio_set_value(S3C2410_GPG(2),0);
			gpio_set_value(S3C2410_GPG(6),1);
			gpio_set_value(S3C2410_GPG(1),1);

			gpio_set_value(S3C2410_GPG(15),0);
			gpio_set_value(S3C2410_GPG(7),1);
			gpio_set_value(S3C2410_GPG(3),1);


			pwm_disable(bz_pwm);
			break;	
		}

		case STOP:
		{
			gpio_set_value(S3C2410_GPG(2),1);
			gpio_set_value(S3C2410_GPG(6),1);
			gpio_set_value(S3C2410_GPG(1),1);

			gpio_set_value(S3C2410_GPG(15),1);
			gpio_set_value(S3C2410_GPG(7),1);
			gpio_set_value(S3C2410_GPG(3),1);


		
			pwm_disable(bz_pwm);

			if(signal_period=='c' || distance_flag==0)
			{	
				pwm_config(bz_pwm, pwm_duty.pulse_width, pwm_duty.period);
				pwm_enable(bz_pwm);
				mdelay(1000);

			}else
			{
				pwm_disable(bz_pwm);
			}
			break;	
		}

		case GO_BACK:
		{
			gpio_set_value(S3C2410_GPG(2),0);
			gpio_set_value(S3C2410_GPG(6),0);
			gpio_set_value(S3C2410_GPG(1),1);

			gpio_set_value(S3C2410_GPG(15),0);
			gpio_set_value(S3C2410_GPG(7),0);
			gpio_set_value(S3C2410_GPG(3),1);
			
			break;
		}


		default:
			return 0;

	}	


	return 0;

}



static int sk_init(void)
{
	
	s3c_gpio_cfgpin(S3C2410_GPB(1), S3C_GPIO_SFN(2));
	s3c_gpio_setpull(S3C2410_GPB(1), S3C_GPIO_PULL_UP);

	printk("SK MODULE is up ...\n");
	if((result=sk_register_cdev())<0)
	{
		return result;
	}
	return 0;
}

static void sk_exit(void)
{	printk("the module is down...\n");
	
	free_irq(IRQ_EINT0,NULL);
	free_irq(IRQ_EINT1,NULL);
	free_irq(IRQ_EINT12, NULL);
	if(ptrmng !=NULL)
	{
		del_timer( &(ptrmng->timer) );
		kfree(ptrmng);
	}

	gpio_free(S3C2410_GPG(4));
	gpio_free(S3C2410_GPG(5));

	cdev_del(&sk_cdev);
	unregister_chrdev_region(sk_dev,1);


}

static int sk_register_cdev(void)
{
	int error;
	if(sk_major){
		sk_dev=MKDEV(sk_major, sk_minor);
		error = register_chrdev_region(sk_dev, 1, "sk");
	}else{
		error = alloc_chrdev_region(&sk_dev, sk_minor,1,"sk");
		sk_major = MAJOR(sk_dev);
	}
	
	if(error <0){
		printk(KERN_WARNING "sk: cant get major %d \n",sk_major);
		return result;
	}
	
	printk("major number = %d \n",sk_major);

	cdev_init(&sk_cdev, &sk_fops);
	sk_cdev.owner = THIS_MODULE;
	sk_cdev.ops = &sk_fops;
	error = cdev_add(&sk_cdev, sk_dev, 1);

	if(error)
		printk(KERN_NOTICE "sk Register Error %d \n",error);
	return 0;
}

module_init(sk_init);
module_exit(sk_exit);
