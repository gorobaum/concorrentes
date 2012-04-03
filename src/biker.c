
#include <stdlib.h>
#include <stdio.h>
#include "biker.h"

static void
init_speed (double speed[3], biker_speed_t speed_type) {
  if (speed_type == UNIFORMSPEED)
    speed[0] = speed[1] = speed[2] = KMH_TO_MMIN(50.0);
  else if (speed_type == RANDOMSPEED) {
    speed[0] = KMH_TO_MMIN(20 + 1.0*rand()/RAND_MAX*60.0);
    speed[1] = KMH_TO_MMIN(20 + 1.0*rand()/RAND_MAX*60.0);
    speed[2] = KMH_TO_MMIN(20 + 1.0*rand()/RAND_MAX*60.0);
  }
  else puts("**WARNING**: Unknown speed type detected.");
}

biker_t*
BIKERmake_all (size_t bikers_num, biker_speed_t speed_type) {
  size_t  i;
  biker_t *bikers = NULL;
  bikers = (biker_t*)malloc(sizeof(*bikers)*bikers_num);
  for (i = 0; i < bikers_num; i++) {
    bikers[i].id = i;
    bikers[i].current_km = -1;
    bikers[i].current_meter = 1000;
    init_speed(bikers[i].speed, speed_type);
  }
  return bikers;
}

