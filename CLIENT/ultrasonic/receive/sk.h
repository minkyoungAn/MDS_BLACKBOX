#ifndef _SK_H_
#define _SK_H_


#define SK_MAGIC 'k'
#define SK_MAXNR 6
//#define SK_LED_OFF _IO(SK_MAGIC,3)
//#define SK_LED_ON _IO(SK_MAGIC,4)

struct pwm_duty_t {
	int pulse_width;		// nsec
	int period;				// nsec
};
/*
#define OFF _IO(SK_MAGIC,0)
#define ON _IO(SK_MAGIC,1)

*/
#define GO_STRAIGHT _IO(SK_MAGIC,0)
#define GO_BACK _IO(SK_MAGIC,1)
#define STOP _IO(SK_MAGIC,2)
//#define PWM_DURATE _IOW(SK_MAGIC,3,struct pwm_duty_t)
#endif /* __SK_H_ */
