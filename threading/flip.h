/*
 * Operating Systems {2INC0} Practical Assignment
 * Threaded application
 *
 * Joris Geurts
 *
 */

#ifndef _FLIP_H_
#define _FLIP_H_

#include "uint128.h"

static void signal_finished();
static void * flip_multiples(void *arg);
static void flip_piece(int n);

typedef struct finished_thread {
  struct finished_thread *next;
  pthread_t thread_id;
} finished_thread_t;

/**
 * NROF_PIECES: size of the board; number of pieces to be flipped
 */
#define NROF_PIECES			3000

/**
 * NROF_THREADS: number of threads that can be run in parallel
 * (value must be between 1 and ... (until you run out of system resources))
 */
#define NROF_THREADS		10

/**
 * buffer[]: datastructure of the pieces; each piece is represented by one bit
 */
static pthread_mutex_t mutexs[(NROF_PIECES/128) + 1];
static uint128_t			buffer [(NROF_PIECES/128) + 1];

#endif
