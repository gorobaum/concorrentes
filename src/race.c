
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "defs.h"
#include "race.h"

static const char SPEEDTYPE_NAMES[2] = { 'U', 'A' };
static const char ROADBLOCKTYPE_NAMES[3] = { 'P', 'S', 'D'};

static simulation_info  info;
static arg_t            args;
static kilometer        *kilometers;
static biker_t          biker;

/*  0 -> success
 * -1 -> failure */
static int
load_simulation_info (const char* filename) {
  FILE    *input;
  char    buffer[BUFFER_SIZE];
  size_t  i, total_length;

  input = fopen(filename, "r");
  if (!input) return -1;
  /* biker num */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%lu", &info.bikers_num);
  /* road capacity */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%lu", &info.road_capacity);
  /* speed type */
  fgets(buffer, BUFFER_SIZE, input);
  switch (buffer[0]) {
    case 'U': info.speed_type = UNIFORMSPEED; break;
    case 'A': info.speed_type = RANDOMSPEED; break;
    default: break;
  }
  /* road length */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%lu", &info.road_total_length);
  /* road blocks */
  for (i = 0, total_length = 0;
       total_length < info.road_total_length && i < MAX_BLOCKS;
       total_length += info.blocks[i++].length) {
    /* block type */
    fgets(buffer, BUFFER_SIZE, input);
    switch (buffer[0]) {
      case 'P': info.blocks[i].type = PLANE; break;
      case 'S': info.blocks[i].type = UP; break;
      case 'D': info.blocks[i].type = DOWN; break;
      default: break;
    }
    /* block length */
    fgets(buffer, BUFFER_SIZE, input);
    sscanf(buffer, "%lu", &info.blocks[i].length);
  }

  info.blocks_num = i;

  fclose(input);
  input = NULL;

  return 0;
}

int
RACEload (const char *inputfile) {
  size_t          i, j, k;

  if (load_simulation_info(inputfile)) {
    puts("NOT");
    return -1;
  }

  biker.current_km = 0;
  biker.current_meter = 0.0;
  for (i = 0; i < 3; i++) biker.speed[i] = 50.0;
  kilometers = malloc(sizeof(kilometer)*info.road_total_length);
  for (i = 0, k = 0; i < info.blocks_num; i++, k = j)
    for (j = k; j < k + info.blocks[i].length; j++) {
      kilometers[j].bikers_num = 0;
      kilometers[j].type = info.blocks[i].type;
      /*printf("KM[%lu] = { %lu, %c }\n",
        j,
        kilometers[j].bikers_num,
        ROADBLOCKTYPE_NAMES[kilometers[j].type]
      );*/
    }

  args.road_total_length = info.road_total_length;
  args.road_capacity = info.road_capacity;
  args.biker = &biker;

  return 0;
}

void
RACEdisplay_info () {
  size_t i;
  printf(
    "%lu\n%lu\n%c\n%lu\n",
    info.bikers_num,
    info.road_capacity,
    SPEEDTYPE_NAMES[info.speed_type],
    info.road_total_length);
  for (i = 0; i < info.blocks_num; i++)
    printf("%c\n%lu\n",
      ROADBLOCKTYPE_NAMES[info.blocks[i].type],
      info.blocks[i].length);
}

int
advance_kilometer (biker_t *biker, size_t road_capacity) {
  while(1) {
    /* LOCK */
    if (kilometers[biker->current_km+1].bikers_num < road_capacity) {
      kilometers[biker->current_km].bikers_num--;
      kilometers[++biker->current_km].bikers_num++;
      break;
    }
    /* UNLOCK */
    /* YIELD */
  }
  /* UNLOCK */
  return 1;
}

void*
biker_callback (void *arg) {
  arg_t   *args = (arg_t*)arg;
  biker_t *biker;
  
  biker = args->biker;
  puts("Biker runs!");
  while (biker->current_km < args->road_total_length) {
    biker->current_meter += 
      biker->speed[kilometers[biker->current_km].type];
    if (biker->current_meter >= 1000.0 ) {
      /*printf("Advanced: %lu\n", biker->current_km);*/
      advance_kilometer (biker, args->road_total_length);
      biker->current_meter = 0.0;
    }
  }

  return NULL;
}

int
RACErun () {
  pthread_t biker_thread;

  if (pthread_create(&biker_thread, NULL, biker_callback, (void*)&args)) {
    printf("error creating thread.");
    return -1;
  }

  puts("Waiting lonely biker.");

  if (pthread_join(biker_thread, NULL)) {
    printf("error joining thread.");
    return -1;
  }

  return 0;
}

void
RACEcleanup () {
  puts("Bye-buh");
  free(kilometers);
}

