#ifndef _SECURE_H_
#define	_SECURE_H_


#define TIME_STEP       (HZ)
#define   ULTRA_SIG           6
#define DEV_SK_MAJOR	241
#define DEV_SK_NAME	"SK"
struct timeval before = {0,}, after = {0,};

typedef struct
{
        struct timer_list  timer;      
} __attribute__ ((packed)) KERNEL_TIMER_MANAGER;
static KERNEL_TIMER_MANAGER *ptrmng = NULL;

int kerneltimer_init(void);
void kerneltimer_exit(void);
void kerneltimer_timeover(unsigned long arg );
void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover );

#endif	/* _SECURE_H_ */
