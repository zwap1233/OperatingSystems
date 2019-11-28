/* 
 * Operating Systems {2INCO} Practical Assignment
 * Interprocess Communication
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
#include <string.h>
#include <errno.h>          // for perror()
#include <unistd.h>         // for getpid()
#include <mqueue.h>         // for mq-stuff
#include <time.h>           // for time()
#include <complex.h>

#include "common.h"
#include "uint128.h"
#include "md5s.h"

char* findkey(uint128_t des_hash, int len, char *str);

static void rsleep (int t);

char begin_letter = 'a';
char end_letter = 'd';

int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the arguments)
    //  * repeatingly:
    //      - read from a message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do that job 
    //      - write the results to a message queue
    //    until there are no more tasks to do
    //  * close the message queues
		
		uint128_t hash = UINT128(0x900150983cd24fb0,0xd6963f7d28e17f72);
		printf ("0x%016lx%016lx\n\n", HI(hash), LO(hash));
		
		char *str = calloc(7,1);
		str[0] = 'a';
		
		char *res = findkey(hash, 1, str);		
		
		if(res != 0){
			printf("%s\n", res);
		}	else {
			printf("failed\n");		
		}

		free(str);

		// FIRST ARG IS REQUEST QEUE SECOND RESPONSE    
    return (0);
}

char* findkey(uint128_t des_hash, int len, char *str){
	uint128_t calc_hash = md5s(str, len);
	if(calc_hash == des_hash){
		return str;
	} else if(len < MAX_MESSAGE_LENGTH) {
		char c;
		for(c = begin_letter; c <= end_letter; c++){
			str[len] = c;
			
			int i;			
			for(i = len+1; i < MAX_MESSAGE_LENGTH; i++){
				str[i] = '\0';			
			}
			
			char *res = findkey(des_hash, len+1, str);
			if(res != 0){
				return str;			
			}
		}
	}
	
	return 0;
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}


