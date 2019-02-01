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

static int ulstasonic_major=0, ulstasonic_minor=0;
static int result;
static dev_t ulstasonic_dev;
static int distance_cm = 0;
static int ultra_cnt = 0;
static struct cdev ulstasonic_cdev;

static irqreturn_t ultra_echo_rising(int irq, void *dev_id, struct pt_regs *regs)
{

  //printk("distance_cm\n");
  if ( gpio_get_value(S3C2410_GPG(2) ) )                  // when rising trigger occured!
  {
    do_gettimeofday(&before); 
  }
  else
  {
    do_gettimeofday(&after);
    
    distance_cm = ((after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec ) ) /58;      

//  printk(" cm : %d \n", distance_cm );
    if ( distance_cm < 15 )
    {
      ultra_cnt++;
  
    }else{
      ultra_cnt=0;
    } 
    memset(&before, 0 , sizeof(struct timeval ) );
    memset(&after, 0 , sizeof(struct timeval ) );
  }

  return IRQ_HANDLED;
}

/*
  ok
*/
void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover )
{
     init_timer( &(pdata->timer) );
     pdata->timer.expires  = get_jiffies_64() + timeover;
     pdata->timer.data     = (unsigned long) pdata      ;
     pdata->timer.function = kerneltimer_timeover       ;
     add_timer( &(pdata->timer) );
}

/*
  ok
*/
void kerneltimer_timeover(unsigned long arg )
{
   KERNEL_TIMER_MANAGER *pdata = NULL;     
	
   
   if( arg )
   {
      pdata = ( KERNEL_TIMER_MANAGER * ) arg;

      /* create pulse to ultrasonic */
      //printk("gpio_set_value 0x%x\n", S3C2410_GPG(1));
      gpio_set_value(S3C2410_GPG(1), 0);
      udelay(20);
      //printk("gpio_set_value = 1\n");
      gpio_set_value(S3C2410_GPG(1), 1);
      udelay(20);
      //printk("gpio_set_value = 0\n");
      gpio_set_value(S3C2410_GPG(1), 0);

      kerneltimer_registertimer( pdata , TIME_STEP );
 }
}

int kerneltimer_init(void)
{
    ptrmng = kmalloc( sizeof( KERNEL_TIMER_MANAGER ), GFP_KERNEL );
    if( ptrmng == NULL ) return -ENOMEM;
     
    memset( ptrmng, 0, sizeof( KERNEL_TIMER_MANAGER ) );
    
    //kerneltimer_registertimer( ptrmng, TIME_STEP );

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

static int ulstasonic_open(struct inode *inode, struct file *filp)
{
  int ret;

  kerneltimer_init();
  
  /* ultrasonic sensor init */
    gpio_request(S3C2410_GPG(1), "TRIG");
    gpio_request(S3C2410_GPG(2), "ECHO");

    s3c_gpio_cfgpin(S3C2410_GPG(1), S3C_GPIO_SFN(1));
    s3c_gpio_cfgpin(S3C2410_GPG(2), S3C_GPIO_SFN(2));

    
  if(request_irq(IRQ_EINT10,(void *)ultra_echo_rising, IRQF_DISABLED|IRQF_TRIGGER_RISING |IRQF_TRIGGER_FALLING, "echoint2",NULL))
  {
    
    ret=-ENOENT;
    printk("failed to request external interrupt microsonar. %d\n",ret);
    return ret;
  }
  kerneltimer_registertimer( ptrmng, TIME_STEP );
  printk("Device has been opened...\n");

  return 0;
}

static int ulstasonic_release(struct inode *inode, struct file *filp)
{ 
//  free_irq(IRQ_EINT0,NULL);
//  free_irq(IRQ_EINT1,NULL);

  printk("device has been closed .. \n");
  return 0;
}

static int ulstasonic_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos)
{
  char data[11];
//  copy_from_user(data,buf,count);
  printk("data>>>>=%s\n",data);
  return count;

}

static int ulstasonic_read(struct file *filp,char *buf, size_t count,loff_t *f_pos)
{
  //char data[20] = "this is read func...";
  int loop;
  char distance_flag;

  if ( ultra_cnt > 5 )
    distance_flag = 0;
  else

    distance_flag = ULTRA_SIG;
  
  for(loop=0; loop<count;loop++)
  {
    put_user(distance_flag,(char *)&buf[0]);

  }
  return 0;
}

struct file_operations ulstasonic_fops = {
  .open = ulstasonic_open,
  .release = ulstasonic_release,
  .write = ulstasonic_write,
  .read = ulstasonic_read
};

static int ulstasonic_init(void)
{
  
  printk("ulstasonic MODULE is up ...\n");
  if((result=ulstasonic_register_cdev())<0)
  {
    return result;
  }
  return 0;
}

static void ulstasonic_exit(void)
{ printk("the module is down...\n");
  
  kerneltimer_exit();

  /* ultrasonic sensor free */
  gpio_free(S3C2410_GPG(1));
  gpio_free(S3C2410_GPG(2));

  free_irq(IRQ_EINT10, NULL);

  cdev_del(&ulstasonic_cdev);
  unregister_chrdev_region(ulstasonic_dev,1);

}


static int ulstasonic_register_cdev(void)
{
  int error;
  if(ulstasonic_major){
    ulstasonic_dev=MKDEV(ulstasonic_major, ulstasonic_minor);
    error = register_chrdev_region(ulstasonic_dev, 1, "ulstasonic");
  }else{
    error = alloc_chrdev_region(&ulstasonic_dev, ulstasonic_minor,1,"ulstasonic");
    ulstasonic_major = MAJOR(ulstasonic_dev);
  }
  
  if(error <0){
    printk(KERN_WARNING "ulstasonic: cant get major %d \n",ulstasonic_major);
    return result;
  }
  
  printk("major number = %d \n",ulstasonic_major);

  cdev_init(&ulstasonic_cdev, &ulstasonic_fops);
  ulstasonic_cdev.owner = THIS_MODULE;
  ulstasonic_cdev.ops = &ulstasonic_fops;
  error = cdev_add(&ulstasonic_cdev, ulstasonic_dev, 1);

  if(error)
    printk(KERN_NOTICE "ulstasonic Register Error %d \n",error);
  return 0;
}

module_init(ulstasonic_init);
module_exit(ulstasonic_exit);

MODULE_LICENSE("Dual BSD/GPL");

