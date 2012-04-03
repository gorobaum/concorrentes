
#ifndef EP1_DEFS_H_
#define EP1_DEFS_H_

#include <pthread.h>

#define BUFFER_SIZE     256
#define MAX_BLOCKS      256

typedef enum {
  UNIFORMSPEED,
  RANDOMSPEED
} biker_speed_t;

typedef struct {
  unsigned  id;
  int       current_km;
  double    current_meter,
            speed[3]; /* METERS/MIN */
} biker_t;

typedef enum {
  PLANE,
  UP,
  DOWN
} roadblock_t;

typedef struct {
  size_t      bikers_num;
  roadblock_t type;
} kilometer;

typedef struct {
  roadblock_t type;
  size_t      length;
} roadblock_info;

typedef struct {
  kilometer         *kilometers;
  pthread_mutex_t   mutex;
  size_t            total_length,
					capacity;
} road_t;

typedef struct {
  int             *ids;
  size_t          last;
  pthread_mutex_t mutex;
} rank_t;

typedef struct {
  biker_t *biker;
  road_t  *road;
  rank_t  *rank;
} arg_t;

typedef struct {
  size_t          bikers_num,
                  road_capacity,
                  road_total_length,
                  blocks_num;
  biker_speed_t   speed_type;
  roadblock_info  blocks[MAX_BLOCKS];
} simulation_info;

#endif /* EP1_DEFS_H_ */

