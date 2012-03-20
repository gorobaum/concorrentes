
#include <stdio.h>

#include "load.h"

int
load_simulation_info (const char* filename, simulation_info *info) {
  FILE    *input;
  char    buffer[BUFFER_SIZE];
  size_t  i, total_length;

  input = fopen(filename, "r");
  if (!input) return -1;
  /* biker num */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%lu", &info->biker_num);
  /* road capacity */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%lu", &info->road_capacity);
  /* speed type */
  fgets(buffer, BUFFER_SIZE, input);
  switch (buffer[0]) {
    case 'U': info->speed_type = UNIFORMSPEED; break;
    case 'A': info->speed_type = RANDOMSPEED; break;
    default: break;
  }
  /* road length */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%lu", &info->road_total_length);
  /* road blocks */
  for (i = 0, total_length = 0;
       total_length < info->road_total_length && i < MAX_BLOCKS;
       total_length += info->blocks[i++].length) {
    /* block type */
    fgets(buffer, BUFFER_SIZE, input);
    switch (buffer[0]) {
      case 'P': info->blocks[i].type = PLANE; break;
      case 'S': info->blocks[i].type = UP; break;
      case 'D': info->blocks[i].type = DOWN; break;
      default: break;
    }
    /* block length */
    fgets(buffer, BUFFER_SIZE, input);
    sscanf(buffer, "%lu", &info->blocks[i].length);
  }

  info->blocks_num = i;

  fclose(input);
  input = NULL;

  return 0;
}

