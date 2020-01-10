/* 
 * Operating Systems {2INCO} Practical Assignment
 * Threaded Application
 *
 * Ivo Kersten 			(1233717)
 * Wouter Schoenmakers 	(1017338)
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

