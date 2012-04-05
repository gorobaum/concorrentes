
#ifndef EP1_DEFS_H_
#define EP1_DEFS_H_

#include <pthread.h>

#define BUFFER_SIZE 256u
#define BLOCKS_NUM  256u

typedef int biker_id;

typedef enum {
  UNIFORMSPEED,
  RANDOMSPEED
} biker_speed_t;

typedef enum {
  PLAIN,
  UP,
  DOWN
} roadblock_t;

typedef struct {
  biker_id  id;
  int       current_km;
  double    current_meter,
            speed[3]; /* METERS/MIN */
  unsigned  score[2]; /* [0] PLAIN SCORE; [1] MOUNTAIN SCORE */
} biker_t;

#define SCORE_TYPE(biker, road) \
  ((road)->kilometers[(biker)->current_km].type != PLAIN)

typedef struct {
  roadblock_t type;
  int         checkpoint_id;
  biker_id    *bikers_id;
} kilometer;

typedef struct {
  roadblock_t type;
  unsigned    length;
} roadblock_info;

typedef struct {
  pthread_mutex_t   mutex;
  biker_id          bikers_id[6];
  double            relative_dist;
  char              complete;
} checkpoint_t;

static const unsigned checkpoint_score[6] = { 45, 35, 25, 15, 10, 5 };

typedef struct {
  kilometer         *kilometers;
  pthread_mutex_t   mutex;
  unsigned          total_length,
					          capacity;
  checkpoint_t      *checkpoints;
} road_t;

typedef struct {
  int             *ids;
  unsigned        last;
  pthread_mutex_t mutex;
} rank_t;

typedef struct {
  biker_t *biker;
  road_t  *road;
  rank_t  *rank;
} arg_t;

typedef struct {
  unsigned        bikers_num,
                  road_capacity,
                  road_total_length,
                  blocks_num;
  biker_speed_t   speed_type;
  roadblock_info  *blocks;
} simulation_info;

#endif /* EP1_DEFS_H_ */

