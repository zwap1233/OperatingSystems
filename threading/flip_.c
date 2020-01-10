<<<<<<< HEAD
/* 
 * Operating Systems {2INCO} Practical Assignment
 * Threaded Application
 *
 * Ivo Kersten 			(1233717)
 * Wouter Schoenmakers 	(1017338)
=======
/*
 * Operating Systems {2INCO} Practical Assignment
 * Threaded Application
 *
 * Wouter Schoenmakers (1017338)
 * Ivo Kersten 			   (1233717)
>>>>>>> e5d12144ab77561fda2e06ce975f68a061562cc0
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * Extra steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
 */
<<<<<<< HEAD
 
=======

>>>>>>> e5d12144ab77561fda2e06ce975f68a061562cc0
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>          // for perror()
#include <pthread.h>

<<<<<<< HEAD
#include "uint128.h"
#include "flip.h"

#define BITMASK(n)			(((uint128_t) 1) << (n))

#define BIT_IS_SET(v,n)		(((v) & BITMASK(n)) == BITMASK(n))

#define BIT_SET(v,n)		((v) = (v) | BITMASK(n))

#define BIT_CLEAR(v,n)		((v) = (v) & ~BITMASK(n))

//Mutexes to lock buffer and count
static pthread_mutex_t mutexbuf 	= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutexcount   = PTHREAD_MUTEX_INITIALIZER;

//Mutex to lock condition
static pthread_mutex_t mutexcond	= PTHREAD_MUTEX_INITIALIZER;

//Condition variable to alert main thread that one of the created threads has finished
static pthread_cond_t  cond			= PTHREAD_COND_INITIALIZER;  

//Counter to keep track of numbe of available threads
int freeThreads = NROF_THREADS;

//Array to keep track of wether or not a certain thread is finished
int status[NROF_THREADS];

//Structure defining input arguments to FlipPieces
typedef struct input_args{
	int i;
	int index;
}input_args;



//Function run by newly created threads
static void* FlipPieces(void *args)
{
	//Cast args to appropriate type
	input_args *input = (input_args *) args;
	int piece, buf, bit;
	
	//Flip every multiple of input->i that fits within NROF_PIECES
	for(int multiple = 1;(piece = multiple*input->i) < NROF_PIECES; multiple++)
	{
		buf = piece/128;
		bit = piece%128;
		//Multiple threads might try to write to buffer, so protect with mutex
		pthread_mutex_lock(&mutexbuf);
		if(BIT_IS_SET(buffer[buf],bit))
			BIT_CLEAR(buffer[buf],bit);
		else
			BIT_SET(buffer[buf],bit);
		pthread_mutex_unlock(&mutexbuf);
	}
	//Indicate thread at index args->index is done
	status[input->index] = 1; 
	//This tread is done, so increase freeThread
	pthread_mutex_lock(&mutexcount);
	freeThreads++;
	pthread_mutex_unlock(&mutexcount);
	//Signal that a thread has finished execution, such that the main thread can wake up if it is blocked
	pthread_cond_signal(&cond);
	return(NULL);
}



int main (void)
{
	int buf, bit, i, j;

	//Array of input arguments to threads
	input_args *args[NROF_THREADS];

	//Array of thread_IDs, initialized to zeros
	pthread_t threads[NROF_THREADS] = {0};
	
	//Initialize array of input arguments, one element per possible thread. Initialize all status elements to available
	for(i = 0; i < NROF_THREADS; i++)
	{
		args[i] = malloc(sizeof(input_args));
		status[i] = 1;
	}
	
	//Loop until all possible divisors have been used
	for(i = 1; i < NROF_PIECES; i++)
	{
		//When no more threads can be created
		while(freeThreads == 0)
		{
			pthread_mutex_lock(&mutexcond);
			pthread_cond_wait(&cond, &mutexcond); //Suspend execution until any thread finishes
			pthread_mutex_unlock(&mutexcond);
		}
		//Find index of next available thread
		for(j = 0; j < NROF_THREADS; j++)
		{
			if(status[j] == 1) //Corresponding thread finished
			{
				if(threads[j] != 0) //If there is a thread to join
				{
					if(pthread_join(threads[j], NULL) != 0) //Join finished thread 
					{
						perror("pthread_join() failed");
						exit(1);
					}
					threads[j] = 0; //Thread is joined, so threads[j] goes back to 0
				}
				status[j] = 0; //No lock needed as no other thread will write to index j now
				break;
			}
		}
		//Prepare input arguments for next thread
		args[j]->i = i;
		args[j]->index = j;
		//Create thread
		if(pthread_create(&threads[j], NULL, FlipPieces, args[j]) != 0)
		{
			perror("pthread_create() failed");
			exit(1);
		}
		//Decrement amount of available threads
		pthread_mutex_lock(&mutexcount); //Other threads might write as well, so protect with mutex
		freeThreads--;
		pthread_mutex_unlock(&mutexcount);
	}
	
	//Once all divisors have been passed to threads, clean up all threads except for main thread
	for(i = 0; i < NROF_THREADS; i++)
	{
		if(threads[i] != 0) //If there is still a thread to join at this index
		{
			if(pthread_join(threads[i],NULL) != 0) //Join thread
			{
				perror("pthread_join() failed");
				exit(1);
			}
		}	
		free(args[i]); //Free allocated memory
	}

	//Print result
	for(int i = 1; i < NROF_PIECES; i++)
	{
		buf = i/128;
		bit = i%128;
		if(BIT_IS_SET(buffer[buf],bit))
			printf("%d\n",i);
	}    
	
    return (0);
}

=======
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
>>>>>>> e5d12144ab77561fda2e06ce975f68a061562cc0
