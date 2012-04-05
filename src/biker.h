
#ifndef EP1_BIKER_H_
#define EP1_BIKER_H_

#include "defs.h"

#define KMH_TO_MMIN(v)  ((v)*1000.0/3600.0)

biker_t*
BIKERmake_all (size_t bikers_num, biker_speed_t speed_type);

void*
BIKERcallback (void *arg);

#endif

