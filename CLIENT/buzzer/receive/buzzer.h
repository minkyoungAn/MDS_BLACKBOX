#ifndef _buzzer_H_
#define _buzzer_H_


#define buzzer_MAGIC 'k'
#define buzzer_MAXNR 6

struct pwm_duty_t {
	int pulse_width;		// nsec
	int period;				// nsec
};

#define BEEP _IO(buzzer_MAGIC,0)
#endif /* __buzzer_H_ */
