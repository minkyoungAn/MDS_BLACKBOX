#ifndef _TH_LED_H_
#define	_TH_LED_H_

typedef struct {
	unsigned long size;
	unsigned char buff[128];
} __attribute__((packed)) sk_info;

#define DEV_LED_MAJOR	241
#define DEV_LED_NAME	"LED"


void led_mknod(void);
void *led_func(void *cmd);


#endif	/* _TH_LED_H_ */

