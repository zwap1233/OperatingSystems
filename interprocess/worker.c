/* 
 * Operating Systems {2INCO} Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (Ivo Kersten)
 * STUDENT_NAME_2 (Wouter Schoenmakers)
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

int findkey(uint128_t des_hash, int len, char *str);

static void rsleep (int t);

mqd_t               mq_fd_request;
mqd_t               mq_fd_response;
MQ_REQUEST_MESSAGE  req;
MQ_RESPONSE_MESSAGE rsp;

char start_letter;
char last_letter;

char 			*mq_name_req;
char 			*mq_name_rsp;

int main (int argc, char * argv[]){
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

  if(argc == 3){
    if(strcmp(argv[0], "worker") == 0){
      mq_name_req = argv[1];
      mq_name_rsp = argv[2];
    } else {
      printf("ERROR: Wrong program name?");
    }
  } else {
    printf("ERROR: Wrong number of arguments\n");
    return -1;
  }

  //printf("Child started with pid: %d, req_que: %s and rsp_que: %s\n", getpid(), mq_name_req, mq_name_rsp);

  mq_fd_request = mq_open(mq_name_req, O_RDONLY);
  mq_fd_response = mq_open(mq_name_rsp, O_WRONLY);

  //printf("Child receiving ...\n");

  char str[7];
  while(1){
    if(mq_receive(mq_fd_request, (char *) &req, sizeof (req), 0) != -1){
      if(req.terminate == 1){
        //printf("Child received request to terminate\n");
        break;
      }

      start_letter = req.start_letter;
      last_letter = req.last_letter;

      memset(&str, 0, 7);
      str[0] = req.letter;

      //printf("Child received request with letter: %c and alphabeth: %c-%c\n", req.letter, start_letter, last_letter);

      int res = findkey(req.hash, 1, str);
      if(res){
        //printf("Child found key: %s\n", str);
        rsp.hash = req.hash;
        strncpy(rsp.string, str, 7);

        while(mq_send (mq_fd_response, (char *) &rsp, sizeof (rsp), 0) == -1){
          if(errno != EAGAIN){
            printf("ERROR: child could not send rsp message\n");
            break;
          }

          sleep(0);
        }
      }	else {
        //printf("Child failed to find key\n");
        //dont give response if it couldnt find the hash
      }
    }

    rsleep(10000);
  }

  //printf("Child shutting down\n");

  mq_close(mq_fd_response);
  mq_close(mq_fd_request);

	// FIRST ARG IS REQUEST QEUE SECOND RESPONSE
  return (0);
}

int findkey(uint128_t des_hash, int len, char *str){
	uint128_t calc_hash = md5s(str, len);
	if(calc_hash == des_hash){
		return 1;
	} else if(len < MAX_MESSAGE_LENGTH) {
		char c;
		for(c = start_letter; c <= last_letter; ++c){
      str[len] = c;

			int i;
			for(i = len+1; i < MAX_MESSAGE_LENGTH; i++){
				str[i] = '\0';
			}

			if(findkey(des_hash, len+1, str)){
        return 1;
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


