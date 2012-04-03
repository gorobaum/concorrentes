
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "defs.h"
#include "biker.h"
#include "race.h"

static const char SPEEDTYPE_NAMES[2] = { 'U', 'A' };
static const char ROADBLOCKTYPE_NAMES[3] = { 'P', 'S', 'D'};

static simulation_info  info;
static arg_t            *args;
static biker_t          *bikers;
static pthread_mutex_t  road_mutex;
static road_t           road;
static size_t           bikers_num;

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

  bikers = BIKERmake_all(info.bikers_num, info.speed_type);
  /*biker.current_km = 0;
  biker.current_meter = 0.0;
  for (i = 0; i < 3; i++) biker.speed[i] = 50.0;*/
  bikers_num = info.bikers_num;
  road.kilometers = malloc(sizeof(kilometer)*info.road_total_length);
  args = malloc(sizeof(arg_t)*info.bikers_num);
  for (i = 0, k = 0; i < info.blocks_num; i++, k = j)
    for (j = k; j < k + info.blocks[i].length; j++) {
      road.kilometers[j].bikers_num = 0;
      road.kilometers[j].type = info.blocks[i].type;
      /*printf("KM[%lu] = { %lu, %c }\n",
        j,
        kilometers[j].bikers_num,
        ROADBLOCKTYPE_NAMES[kilometers[j].type]
      );*/
    }

  road.total_length = info.road_total_length;
  road.capacity = info.road_capacity;
  for (i = 0; i < info.bikers_num; i++) {
    args[i].biker = &bikers[i];
	args[i].road = &road;
  }

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
advance_kilometer (biker_t *biker, road_t *road) {
  while(1) {
    /* LOCK */
    pthread_mutex_lock(&road->mutex);
    if (road->kilometers[biker->current_km+1].bikers_num < road->capacity) {
      road->kilometers[biker->current_km].bikers_num--;
      road->kilometers[++biker->current_km].bikers_num++;
      break;
    }
    /* UNLOCK */
    pthread_mutex_unlock(&road->mutex);
    /* YIELD */
    /*sched_yield();*/
  }
  /* UNLOCK */
  pthread_mutex_unlock(&road->mutex);
  return 1;
}

void*
biker_callback (void *arg) {
  arg_t   *args = (arg_t*)arg;
  road_t  *road = args->road;
  biker_t *biker = args->biker;
  
  puts("Biker runs!");
  while (biker->current_km < road->total_length) {
    biker->current_meter += 
      biker->speed[road->kilometers[biker->current_km].type];
    if (biker->current_meter >= 1000.0 ) {
      /*printf("Advanced: %lu\n", biker->current_km);*/
      advance_kilometer (biker, road);
      biker->current_meter = 0.0;
    }
  }

  return NULL;
}

int
RACErun () {
  size_t i;
  pthread_t *biker_thread;

  biker_thread = malloc(sizeof(pthread_t)*bikers_num);

  if (pthread_mutex_init(&road_mutex, NULL)) {
    puts("error creating mutex.");
    return -1;
  }
  
  for (i = 0; i < bikers_num; i++) { 
    if (pthread_create(&biker_thread[i], NULL, biker_callback, (void*)&args[i])) {
      puts("error creating thread.");
      return -1;
    }
  }

  puts("Waiting lonely biker.");

  for (i = 0; i < bikers_num; i++) {
    if (pthread_join(biker_thread[i], NULL)) {
      printf("error joining thread.");
      return -1;
    }
  }

  free(biker_thread);
  pthread_mutex_destroy(&road_mutex);

  return 0;
}

void
RACEcleanup () {
  puts("Bye-buh");
  free(bikers);
  free(road.kilometers);
  free(args);
}

