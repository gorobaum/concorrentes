
#ifndef EP1_LOAD_H_
#define EP1_LOAD_H_

#include "defs.h"

/* -0 -> success
 * -1 -> failure */
int
load_simulation_info (const char *filename, simulation_info *info);

#endif

