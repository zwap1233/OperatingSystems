/*
 * Operating Systems {2INCO} Practical Assignment
 * Threaded Application
 *
 * Wouter Schoenmakers (1017338)
 * Ivo Kersten 			   (1233717)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * Extra steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>          // for perror()
#include <pthread.h>

#include "flip.h"

static void signal_finished();
static void * flip_multiples(void *arg);
static void flip_piece(int n);

typedef struct finished_thread {
  struct finished_thread *next;
  pthread_t thread_id;
} finished_thread_t;

static pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
finished_thread_t *start = NULL;
finished_thread_t *last = NULL;

int main (void){
  // TODO: start threads to flip the pieces and output the results
  // (see thread_test() and thread_mutex_test() how to use threads and mutexes,
  //  see bit_test() how to manipulate bits in a large integer)

  pthread_t thread_id;

  //init buffers
  int i;
  char *buf = (char *) buffer;
  for(i = 0; i < (NROF_PIECES/128 +1)*16; ++i){
    buf[i] = 0xff;
  }
  
  /*for(i = 0; i < (NROF_PIECES/128 +1); ++i){
    printf("0x%016lx,0x%016lx\n", HI(buffer[i]), LO(buffer[i]));
  }
  printf("\n");*/


  //create threads and distribute jobs
  int threads = 0;
  for(i = 2; i < NROF_PIECES; i=i){
    while(threads < NROF_THREADS && i < NROF_PIECES){

      int *parm_n = malloc(sizeof(int));
      *parm_n = i;
      pthread_create(&thread_id, NULL, flip_multiples, parm_n);

      ++threads;
      ++i;
    }
    
    while(start == NULL){
			pthread_mutex_lock(&cond_mutex);
			pthread_cond_wait(&cond_var, &cond_mutex);
			pthread_mutex_unlock(&cond_mutex);
		}

    pthread_mutex_lock(&list_mutex);
    if(start != NULL){
      pthread_join(start->thread_id, NULL);
      finished_thread_t *next_start = start->next;

      if(next_start == NULL){
        last = NULL;
      }

      free(start);
      start = next_start;

      --threads;
    }
    pthread_mutex_unlock(&list_mutex);
  }

  //join remaining threads
  while(start != NULL || threads > 0){
    if(start != NULL){
      pthread_mutex_lock(&list_mutex);

      pthread_join(start->thread_id, NULL);
      finished_thread_t *next_start = start->next;

      if(next_start == NULL){
        last = NULL;
      }

      free(start);
      start = next_start;
      pthread_mutex_unlock(&list_mutex);

      --threads;
    }
  }

  //printing the results
  for(i = 1; i < NROF_PIECES; ++i){
    int buf_num = i / 128;
    int bit_num = i % 128;

    if((buffer[buf_num] & (((uint128_t) 0x1) << bit_num)) > 0){
      printf("%d\n", i);
    }
  }

  return (0);
}

static void signal_finished(){
  pthread_mutex_lock(&list_mutex);

  if(last == NULL){
    last = malloc(sizeof(finished_thread_t));
    last->thread_id = pthread_self();
    last->next = NULL;
    start = last;
  } else {
    finished_thread_t *item = malloc(sizeof(finished_thread_t));
    item->thread_id = pthread_self();
    item->next = NULL;

    last->next = item;
    last = item;
  }

  pthread_mutex_unlock(&list_mutex);

	pthread_cond_signal(&cond_var);
}

static void * flip_multiples(void *arg){
  int n = *((int *) arg);

  int i;
  for(i = 1; i*n <= NROF_PIECES; ++i){
    flip_piece(i*n);
  }

  free(arg);

  signal_finished();

  return NULL;
}

static void flip_piece(int n){
  if(n <= 0 || n > NROF_PIECES){
    fprintf(stderr, "Cant flip %d, out of bounds\n", n);
    return;
  }

  int buf_num = n / 128;
  int bit_num = n % 128;
	
  pthread_mutex_lock(&buffer_mutex);

  if((buffer[buf_num] & (((uint128_t) 0x1) << bit_num)) > 0){
    buffer[buf_num] = buffer[buf_num] & ~(((uint128_t) 0x1) << bit_num); //set low
  } else {
    buffer[buf_num] = buffer[buf_num] | (((uint128_t) 0x1) << bit_num); //set high
  }

  pthread_mutex_unlock(&buffer_mutex);
}
