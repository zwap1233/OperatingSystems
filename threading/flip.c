/*
 * Operating Systems {2INCO} Practical Assignment
 * Threaded Application
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
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

#include "uint128.h"
#include "flip.h"

static void * flip_multiples(void *arg);
static void flip_piece(int n);

int main (void){
  // TODO: start threads to flip the pieces and output the results
  // (see thread_test() and thread_mutex_test() how to use threads and mutexes,
  //  see bit_test() how to manipulate bits in a large integer)

  int *parm_n;
  pthread_t thread_id;

  parm_n = malloc(sizeof(int));

  int i;
  char *buf = (char *) buffer;
  for(i = 0; i < (NROF_PIECES/128 +1)*16; ++i){
    buf[i] = 0xff;
  }

  /*for(i = 0; i < (NROF_PIECES/128 +1); ++i){
    printf("0x%016lx,0x%016lx\n", HI(buffer[i]), LO(buffer[i]));
  }
  printf("\n");*/

  for(i = 1; i <= NROF_PIECES; ++i){
    *parm_n = i;
    pthread_create(&thread_id, NULL, flip_multiples, parm_n);
    //sleep(0);
    pthread_join(thread_id, NULL);
  }

  for(i = 0; i < NROF_PIECES; ++i){
    int buf_num = i / 128;
    int bit_num = i % 128;

    if((buffer[buf_num] & (((uint128_t) 0x1) << bit_num)) == 0){
      printf("%d\n", i);
    }
  }

  return (0);
}

static void * flip_multiples(void *arg){
  int n = *((int *) arg);

  int i;
  for(i = 1; i*n <= NROF_PIECES; ++i){
    flip_piece(i*n);
  }

  return NULL;
}

static void flip_piece(int n){
  if(n <= 0 || n > NROF_PIECES){
    fprintf(stderr, "Cant flip %d, out of bounds\n", n);
    return;
  }

  int buf_num = n / 128;
  int bit_num = n % 128;

  if((buffer[buf_num] & (((uint128_t) 0x1) << bit_num)) > 0){
    buffer[buf_num] = buffer[buf_num] & ~(((uint128_t) 0x1) << bit_num); //set low
  } else {
    buffer[buf_num] = buffer[buf_num] | (((uint128_t) 0x1) << bit_num); //set high
  }
}
