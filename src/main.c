
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "defs.h"
#include "load.h"

static char SPEEDTYPE_NAMES[2] = { 'U', 'A' };

static char ROADBLOCKTYPE_NAMES[3] = { 'P', 'S', 'D'};

static kilometer *kilometers;

void dump_simulation_info (const simulation_info *info) {
  size_t i;
  printf(
    "%lu\n%lu\n%c\n%lu\n",
    info->bikers_num,
    info->road_capacity,
    SPEEDTYPE_NAMES[info->speed_type],
    info->road_total_length);
  for (i = 0; i < info->blocks_num; i++)
    printf("%c\n%lu\n",
      ROADBLOCKTYPE_NAMES[info->blocks[i].type],
      info->blocks[i].length);
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
  while (biker->current_km <= args->road_total_length) {
    biker->current_meter += 
      biker->speed[kilometers[biker->current_km].type];
    if (biker->current_meter >= 1000.0 ) {
      advance_kilometer (biker, args->road_total_length);
      biker->current_meter = 0.0;
    }
  }

  return NULL;
}

int
main (int argc, char **argv) {
  int i;
  pthread_t       biker_thread;
  simulation_info info;
  arg_t           args;
  biker_t         biker;

  if (argc < 2) {
    puts("-.-");
    return EXIT_FAILURE;
  }

  if (load_simulation_info(argv[1], &info)) {
    puts("NOT");
    return EXIT_FAILURE;
  }
  
  biker.current_km = 0;
  biker.current_meter = 0.0;
  for (i = 0; i < 3; i++) biker.speed[i] = 50.0;
  kilometers = malloc(sizeof(kilometer)*info.road_total_length);
  args.road_total_length = info.road_total_length;
  args.road_capacity = info.road_capacity;
  args.biker = &biker;
  
  dump_simulation_info(&info);

  if (pthread_create(&biker_thread, NULL, biker_callback, (void*)&args)) {
    printf("error creating thread.");
    return EXIT_FAILURE;
  }

  if (pthread_join(biker_thread, NULL)) {
    printf("error joining thread.");
    return EXIT_FAILURE;
  }


  free(kilometers);
  return EXIT_SUCCESS;

}

