
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "defs.h"
#include "load.h"

static char SPEEDTYPE_NAMES[2] = { 'U', 'A' };

static char ROADBLOCKTYPE_NAMES[3] = { 'P', 'S', 'D'};

void dump_simulation_info (const simulation_info *info) {
  size_t i;
  printf(
    "%lu\n%lu\n%c\n%lu\n",
    info->biker_num,
    info->road_capacity,
    SPEEDTYPE_NAMES[info->speed_type],
    info->road_total_length);
  for (i = 0; i < info->blocks_num; i++)
    printf("%c\n%lu\n",
      ROADBLOCKTYPE_NAMES[info->blocks[i].type],
      info->blocks[i].length);
}

void*
thread_function (void *arg) {
  int i;
  for (i=0; i<20; i++) {
    printf("Thread says hi!\n");
    sleep(1);
  }
  return NULL;
}

int
main (int argc, char **argv) {

  pthread_t       mythread;
  simulation_info info;

  if (argc < 2) {
    puts("-.-");
    return EXIT_FAILURE;
  }

  if (load_simulation_info(argv[1], &info)) {
    puts("NOT");
    return EXIT_FAILURE;
  }

  dump_simulation_info(&info);

  if (pthread_create(&mythread, NULL, thread_function, NULL)) {
    printf("error creating thread.");
    return EXIT_FAILURE;
  }

  if (pthread_join(mythread, NULL)) {
    printf("error joining thread.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

}

