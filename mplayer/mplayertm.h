#ifndef __MPLAYER_TM__
#define __MPLAYER_TM__

//#include <sys/time.h>
#include <time.h>
#undef time
time_t time(time_t* t);
#undef clock
clock_t clock(void);
#undef usleep
#define usleep(x) usec_sleep(x)
int usec_sleep(int time);
#endif