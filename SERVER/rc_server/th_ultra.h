#ifndef __TH_ULTRA_H__
#define __TH_ULTRA_H__

#define	ULTRA_SIG           6
#define DEV_ULTRA_NAME  	"ultrasonic"
#define	DEV_ULTRA_MAJOR 	103

void ultra_mknod(void);
void *ultra_func(void *data);

#endif //__TH_ULTRA_H__