#ifndef _PWM_H_
#define _PWM_H_

struct pwm_pulse{  
	unsigned long width;
	unsigned long period;
};

#define DEV_PWM_NAME	"PWM"
#define DEV_PWM_MAJOR	100
#define FRONT			1
#define BACK			8
#define LEFTPWM			3
#define RIGHTPWM		4
#define LEFT			5
#define RIGHT			6
#define STOP			7

#endif /* _PWM_H_ */
