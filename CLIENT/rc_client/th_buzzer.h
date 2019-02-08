#ifndef __TH_BUZZER_H__
#define __TH_BUZZER_H__

#define	BUZZER_SIG           6
#define DEV_BUZZER_NAME  	"buzzer"
#define	DEV_BUZZER_MAJOR 	102

void buzzer_mknod(void);
void *buzzer_func(int fd);

#endif //__TH_BUZZER_H__
