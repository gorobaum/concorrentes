
#ifndef EP1_DEFS_H_
#define EP1_DEFS_H_

#define BUFFER_SIZE 256
#define MAX_BLOCKS  256

typedef enum {
  UNIFORMSPEED,
  RANDOMSPEED
} biker_speed_t;

char SPEEDTYPE_NAMES[2] = { 'U', 'A' };

typedef enum {
  PLANE,
  UP,
  DOWN
} roadblock_t;

char ROADBLOCKTYPE_NAMES[3] = { 'P', 'S', 'D'};

typedef struct {
  roadblock_t type;
  size_t      length;
} roadblock_info;

typedef struct {
  size_t          biker_num,
                  road_capacity,
                  road_total_length,
                  blocks_num;
  biker_speed_t   speed_type;
  roadblock_info  blocks[MAX_BLOCKS];
} simulation_info;

#endif /* EP1_DEFS_H_ */

