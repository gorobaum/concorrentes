
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "biker.h"

static struct timespec delay = { 0, 500000 };

/* Funções de Inicialização */

static void
init_speed (double speed[3], biker_speed_t speed_type) {
  if (speed_type == UNIFORMSPEED)
    speed[0] = speed[1] = speed[2] = KMH_TO_MMIN(50.0);
  else if (speed_type == RANDOMSPEED) {
    speed[0] = KMH_TO_MMIN(20 + 1.0*rand()/RAND_MAX*60.0);
    speed[1] = KMH_TO_MMIN(20 + 1.0*rand()/RAND_MAX*60.0);
    speed[2] = KMH_TO_MMIN(20 + 1.0*rand()/RAND_MAX*60.0);
  }
  else puts("**WARNING**: Unknown speed type detected.");
}

biker_t*
BIKERmake_all (size_t bikers_num, biker_speed_t speed_type) {
  int  i;
  biker_t *bikers = NULL;
  bikers = (biker_t*)malloc(sizeof(*bikers)*bikers_num);
  for (i = 0; i < bikers_num; i++) {
    bikers[i].id = i;
    bikers[i].current_km = -1;
    bikers[i].current_meter = 1000.0;
    init_speed(bikers[i].speed, speed_type);
    printf("speed #%u: %f\n", i, bikers[i].speed[0]);
  }
  return bikers;
}

/* Funções da simulação dos bikers. */

static int
advance_kilometer (biker_t *biker, road_t *road) {
  while(1) {
    /* LOCK */
    pthread_mutex_lock(&road->mutex);
    if (biker->current_km >= (int)road->total_length-1) {
      road->kilometers[biker->current_km].bikers_num--;
      biker->current_km++;
      break;
    }
    else if (road->kilometers[biker->current_km+1].bikers_num < road->capacity) {
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

static void
finish_race (biker_t *biker, rank_t *rank) {
  pthread_mutex_lock(&rank->mutex);
  rank->ids[rank->last++] = biker->id;
  pthread_mutex_unlock(&rank->mutex);
}

void
checkpoint (biker_t *biker, road_t *road) {
  int check_id, i;
  check_id = road->kilometers[biker->current_km].checkpoint_id;
  if (check_id >= 0) {
    pthread_mutex_lock(&road->checkpoints[check_id].mutex);
    if (biker->current_meter >= road->checkpoints[check_id].relative_dist) {
      for (i = 0; i < 6; i++) {
        if (road->checkpoints[check_id].bikers_id[i] == biker->id) break;
        if (road->checkpoints[check_id].bikers_id[i] < 0)  {
          /*printf("Biker #%d has crossed cp #%d\n", biker->id, check_id);*/
          road->checkpoints[check_id].bikers_id[i] = biker->id;
          break;
        }
      }
      if (i == 5 && !road->checkpoints[check_id].complete) {
        printf("Checkpoint #%d:\n", check_id);
        for (i = 0; i < 6; i++)
          printf("\t[%d] biker #%d\n", i, road->checkpoints[check_id].bikers_id[i]);
        road->checkpoints[check_id].complete = 1;
      }
    pthread_mutex_unlock(&road->checkpoints[check_id].mutex);
    }
  }
}

void*
BIKERcallback (void *arg) {
  arg_t   *args = (arg_t*)arg;
  road_t  *road = args->road;
  biker_t *biker = args->biker;
  rank_t  *rank = args->rank;
  
  /*puts("Biker runs!");*/
  while (biker->current_km < (int)road->total_length) {
    if (biker->current_meter >= 1000.0 ) {
      /*printf("Advanced: %lu\n", biker->current_km);*/
      advance_kilometer (biker, road);
      biker->current_meter -= 1000.0;
      /*printf("Biker #%u advanced to km %u.\n", biker->id, biker->current_km);*/
    }
    else {
      biker->current_meter += 
        biker->speed[road->kilometers[biker->current_km].type]/2.0;
      checkpoint(biker, road);
    }
    nanosleep(&delay, NULL);
  }
  finish_race(biker, rank);
  /*printf("Biker #%u finished.\n", biker->id);*/
  return NULL;
}

