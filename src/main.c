
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "defs.h"

void dump_simulation_info (const simulation_info *info) {
  printf(
    "%lu\n%lu\n%c\n%lu\n",
    info->biker_num,
    info->road_capacity,
    SPEEDTYPE_NAMES[info->speed_type],
    info->road_total_length);
}

/* 0  -> success
 * -1 -> failure */
int
load_simulation_info (const char* filename, simulation_info *info) {
  FILE *input;
  char buffer[BUFFER_SIZE];

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
  switch(buffer[0]) {
    case 'U': info->speed_type = UNIFORMSPEED; break;
    case 'A': info->speed_type = RANDOMSPEED; break;
    default: break;
  }
  /* road length */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%lu", &info->road_total_length);

  fclose(input);
  input = NULL;

  return 0;
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

