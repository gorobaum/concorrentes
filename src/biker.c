
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
    bikers[i].score[0] = bikers[i].score[1] = 0;
    printf("speed #%u: %f\n", i, bikers[i].speed[0]);
  }
  return bikers;
}

/* Funções da simulação dos bikers. */

/* REQUIRES LOCK */
static void 
remove_from_km (biker_t *biker, kilometer *km, size_t cap) {
  int i;
  for (i = 0; i < cap; i++)
    if (km->bikers_id[i] == biker->id) {
      km->bikers_id[i] = -1;
      return;
    }
}

/* REQUIRES LOCK */
static int
add_to_km (biker_t *biker, kilometer *km, size_t cap) {
  int i;
  for (i = 0; i < cap; i++)
    if (km->bikers_id[i] == -1) {
      km->bikers_id[i] = biker->id;
      return 1;
    }
  return 0;
}

static void
advance_kilometer (biker_t *biker, road_t *road) {
  while(1) {
    /* LOCK */
    pthread_mutex_lock(&road->mutex);
    if (biker->current_km < 0) {
      /*road->kilometers[++biker->current_km].bikers_num++;*/
      if (add_to_km(biker, &road->kilometers[biker->current_km+1], road->capacity)) {
        biker->current_km++;
        break;
      }
    }
    if (biker->current_km >= (int)road->total_length-1) {
      /*road->kilometers[biker->current_km].bikers_num--;*/
      remove_from_km(biker, &road->kilometers[biker->current_km], road->capacity);
      biker->current_km++;
      break;
    }
    else if (add_to_km(biker, &road->kilometers[biker->current_km+1], road->capacity)) {
      /*road->kilometers[biker->current_km].bikers_num--;
      road->kilometers[++biker->current_km].bikers_num++;*/
      remove_from_km(biker, &road->kilometers[biker->current_km], road->capacity);
      biker->current_km++;
      break;
    }
    /* UNLOCK */
    pthread_mutex_unlock(&road->mutex);
    /* YIELD */
    /*sched_yield();*/
  }
  /* UNLOCK */
  pthread_mutex_unlock(&road->mutex);
}

void
check_checkpoint (biker_t *biker, road_t *road) {
  int           check_id, i;
  checkpoint_t  *checkpoint;
  check_id = road->kilometers[biker->current_km].checkpoint_id;
  if (check_id >= 0) {
    checkpoint = &road->checkpoints[check_id];
    pthread_mutex_lock(&checkpoint->mutex);
    if (biker->current_meter >= checkpoint->relative_dist) {
      for (i = 0; i < 6; i++) {
        if (checkpoint->bikers_id[i] == biker->id) break;
        if (checkpoint->bikers_id[i] < 0)  {
          /*printf("Biker #%d has crossed cp #%d\n", biker->id, check_id);*/
          checkpoint->bikers_id[i] = biker->id;
          biker->score[SCORE_TYPE(biker, road)] += checkpoint_score[i];
          break;
        }
      }
      if (i == 2 && !checkpoint->complete) {
        printf(
          "Checkpoint #%d at %.1fkm:\n",
          check_id,
          biker->current_km + checkpoint->relative_dist/1000.0
        );
        for (i = 0; i < 3; i++)
          printf(
            "\t[%d] biker #%02d (+%upts)\n",
            i+1,
            checkpoint->bikers_id[i],
            checkpoint_score[i]);
        checkpoint->complete = 1;
      }
    }
    pthread_mutex_unlock(&checkpoint->mutex);
  }
}

static void
finish_race (biker_t *biker, rank_t *rank) {
  pthread_mutex_lock(&rank->mutex);
  rank->ids[rank->last++] = biker->id;
  pthread_mutex_unlock(&rank->mutex);
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
      check_checkpoint(biker, road);
    }
    nanosleep(&delay, NULL);
  }
  finish_race(biker, rank);
  /*printf("Biker #%u finished.\n", biker->id);*/
  return NULL;
}

