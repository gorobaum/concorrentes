
#ifndef EP1_DEFS_H_
#define EP1_DEFS_H_

#define BUFFER_SIZE 256
#define MAX_BLOCKS  256

typedef enum {
  UNIFORMSPEED,
  RANDOMSPEED
} biker_speed_t;

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
  size_t  current_km;
  double  current_meter,
          speed[3]; /* METERS/MIN */
} biker_t;

typedef struct {
  biker_t *biker;
  size_t  road_total_length,
          road_capacity;
} arg_t;

typedef struct {
  roadblock_t type;
  size_t      length;
} roadblock_info;

typedef struct {
  size_t          bikers_num,
                  road_capacity,
                  road_total_length,
                  blocks_num;
  biker_speed_t   speed_type;
  roadblock_info  blocks[MAX_BLOCKS];
} simulation_info;

#endif /* EP1_DEFS_H_ */

