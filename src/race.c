
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "defs.h"
#include "biker.h"
#include "race.h"

static const char SPEEDTYPE_NAMES[2] = { 'U', 'A' };
static const char ROADBLOCKTYPE_NAMES[3] = { 'P', 'S', 'D'};

/* This contains the basic information about the current simulation. */
static simulation_info  info;
/* This is an array of arg_t structures used by the biker threads. */
static arg_t            *args;
/* This is an array of structures representing each biker. */
static biker_t          *bikers;
/* This is our "estrada" array. It has as many positions as there are kilometers
 * in the current race being simulated. At any given time, its i-th position
 * stores the identifiers of all the bikers in the interval [i,i+1). */
static road_t           road;
static rank_t           rank;

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
  sscanf(buffer, "%u", &info.bikers_num);
  /* road capacity */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%u", &info.road_capacity);
  /* speed type */
  fgets(buffer, BUFFER_SIZE, input);
  switch (buffer[0]) {
    case 'U': info.speed_type = UNIFORMSPEED; break;
    case 'A': info.speed_type = RANDOMSPEED; break;
    default: break;
  }
  /* road length */
  fgets(buffer, BUFFER_SIZE, input);
  sscanf(buffer, "%u", &info.road_total_length);
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
    sscanf(buffer, "%u", &info.blocks[i].length);
  }

  info.blocks_num = i;

  fclose(input);
  input = NULL;

  return 0;
}

void
RACEcreate_checkpoint() {
  int i, j, check_num;
  size_t actual_km = 0;

  road.checkpoints = malloc(sizeof(*road.checkpoints)*info.blocks_num);

  for (i = 0, check_num = 0; i < info.blocks_num; i++) {
    if (info.blocks[i].type != UP) {
      road.kilometers[actual_km].checkpoint_id = check_num;
      road.checkpoints[check_num].complete = 0;
      for (j = 0; j < 6; j++)
        road.checkpoints[check_num].bikers_id[j] = -1;
      road.checkpoints[check_num++].relative_dist =
        (info.blocks[i].type==PLANE)*(info.blocks[i].length%2)*500.0;
    }
    actual_km += info.blocks[i].length; 
  }
}

static void
init_km(kilometer *km, roadblock_t type) {
  int i;
  /*km->bikers_num = 0;*/
  km->type = type;
  km->checkpoint_id = -1;
  km->bikers_id = malloc(sizeof(*km->bikers_id)*info.road_capacity);
  for (i = 0; i < info.road_capacity; i++)
    km->bikers_id[i] = -1;
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
  road.kilometers = malloc(sizeof(kilometer)*info.road_total_length);
  args = malloc(sizeof(arg_t)*info.bikers_num);
  rank.ids = malloc(sizeof(*rank.ids)*info.bikers_num);
  rank.last = 0;
  for (i = 0, k = 0; i < info.blocks_num; i++, k = j)
    for (j = k; j < k + info.blocks[i].length; j++) {
      init_km(&road.kilometers[j], info.blocks[i].type);
      /*printf("KM[%lu] = { %lu, %c }\n",
        j,
        kilometers[j].bikers_num,
        ROADBLOCKTYPE_NAMES[kilometers[j].type]
      );*/
    }
  
  RACEcreate_checkpoint();

  road.total_length = info.road_total_length;
  road.capacity = info.road_capacity;
  for (i = 0; i < info.bikers_num; i++) {
    args[i].biker = &bikers[i];
	  args[i].road = &road;
    args[i].rank = &rank;
  }

  return 0;
}

void
RACEdisplay_info () {
  size_t i;
  printf(
    "%u\n%u\n%c\n%u\n",
    info.bikers_num,
    info.road_capacity,
    SPEEDTYPE_NAMES[info.speed_type],
    info.road_total_length);
  for (i = 0; i < info.blocks_num; i++)
    printf("%c\n%u\n",
      ROADBLOCKTYPE_NAMES[info.blocks[i].type],
      info.blocks[i].length);
}

int
RACErun () {
  size_t i;
  pthread_t *biker_thread;

  biker_thread = malloc(sizeof(pthread_t)*info.bikers_num);

  if (pthread_mutex_init(&road.mutex, NULL)) {
    puts("error creating mutex.");
    return -1;
  }

  if (pthread_mutex_init(&rank.mutex, NULL)) {
    puts("error creating mutex.");
    return -1;
  }

  for (i = 0; i < info.blocks_num; i++)
    if (pthread_mutex_init(&road.checkpoints[i].mutex, NULL)) {
      puts("error creating checkpoints mutex.");
      return -1;
    }
  
  for (i = 0; i < info.bikers_num; i++)
    if (pthread_create(&biker_thread[i], NULL, BIKERcallback, (void*)&args[i]))
      printf("**WARNING**: Biker #%u was not able to enter the race.\n", i);

  for (i = 0; i < info.bikers_num; i++)
    if (pthread_join(biker_thread[i], NULL))
      printf("**WARNING**: Biker #%u's companions miss him.\n", i);

  free(biker_thread);
  pthread_mutex_destroy(&road.mutex);
  pthread_mutex_destroy(&rank.mutex);

  puts("======= YELLOW RANKING =======");
  for (i = 0; i < info.bikers_num; i++)
    printf("\t[%3u] Biker #%d\n", i+1, rank.ids[i]);

  return 0;
}

void
RACEcleanup () {
  int i;
  puts("Bye-buh");
  free(bikers);
  for (i = 0; i < road.total_length; i++)
    free(road.kilometers[i].bikers_id);
  free(road.kilometers);
  free(road.checkpoints);
  free(args);
  free(rank.ids);
}

