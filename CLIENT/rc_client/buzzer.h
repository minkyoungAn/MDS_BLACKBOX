#ifndef _buzzer_H_
#define _buzzer_H_

#define	BUZZER_SIG           6

struct pwm_duty_t 
{
	int pulse_width;		// nsec
	int period;				// nsec
};

struct pwm_device 
{
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
static struct pwm_duty_t pwm_duty;
static struct cdev buzzer_cdev;

static int buzzer_register_cdev(void);

#endif /* __buzzer_H_ */
