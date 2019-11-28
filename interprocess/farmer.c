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
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>         // for execlp
#include <mqueue.h>         // for mq

#include "settings.h"
#include "common.h"

#define STUDENT_NAME "IvoKersten"

void CreateWorkers(char mq_name1[], char mq_name2[]);
void Farm(mqd_t mq_fd_request, mqd_t mq_fd_response);
void ConstructMessage(int hashindex, char startletter);
bool SendMessage(mqd_t mq_fd_request);
bool ReadMessage(mqd_t mq_fd_response);
void PrintOriginal();


MQ_REQUEST_MESSAGE 	req;
MQ_RESPONSE_MESSAGE rsp;
char *original[MD5_LIST_NROF]; //list of original strings

int main (int argc, char * argv[]){
	if (argc != 1)
	{
	  fprintf (stderr, "%s: invalid arguments\n", argv[0]);
	}
	char 			mq_name1[80];
	char 			mq_name2[80];
	mqd_t			mq_fd_request;
	mqd_t			mq_fd_response;
	struct mq_attr	attr;

	//Generate message queue names
	sprintf(mq_name1,"/mq_request_%s_%d",STUDENT_NAME,getpid());
	sprintf(mq_name2,"/mq_response_%s_%d",STUDENT_NAME,getpid());

	//Create request message queue
	attr.mq_maxmsg = MQ_MAX_MESSAGES;
	attr.mq_msgsize = sizeof(MQ_REQUEST_MESSAGE);
	mq_fd_request = mq_open(mq_name1, O_WRONLY | O_CREAT | O_EXCL | O_NONBLOCK, 0600, &attr);

	//Create response message queue
	attr.mq_msgsize = sizeof(MQ_RESPONSE_MESSAGE);
	mq_fd_response = mq_open(mq_name2, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, 0600, &attr);

	//Create workers
	CreateWorkers(mq_name1, mq_name2);

	//Do the farming
	Farm(mq_fd_request, mq_fd_response);

	//Wait for all child processes to finish
	for(int i = 0; i < NROF_WORKERS; i++)
	{
		wait(NULL);
	}

	//Print original strings
	PrintOriginal();

	//Clean up message queues
	mq_close(mq_fd_request);
	mq_close(mq_fd_response);
	mq_unlink(mq_name1);
	mq_unlink(mq_name2);
	return (0);
}

//Prints the original strings to the standard output in the correct order
void PrintOriginal()
{
	for(int i = 0; i < MD5_LIST_NROF; i++)
	{
		printf("%s\n",original[i]);
		free(original[i]);
	}
}

void Farm(mqd_t mq_fd_request, mqd_t mq_fd_response)
{
	bool msgsent = true, msgread;
	int messageCount = 0, receivedMessageCount = 0;
	int hashindex;
	char startletter;

	while(messageCount <= (JOBS_NROF+NROF_WORKERS))
	{
		msgread = false;
		//If a message was sent, construct the next message
		if(msgsent)
		{
			if(messageCount < JOBS_NROF) //If there are more jobs to hand out, construct messages
			{
				hashindex = messageCount / ALPHABET_NROF_CHAR;
				startletter = ALPHABET_START_CHAR + (messageCount % ALPHABET_NROF_CHAR);
				ConstructMessage(hashindex, startletter);
			}
			else //Else, construct termination message
				req.terminate = 1;
			messageCount++;
		}

		//Try to send a new message to the request queue and read a message from the response queue
		msgsent = SendMessage(mq_fd_request);
		msgread = ReadMessage(mq_fd_response);

		if(msgread)
			receivedMessageCount++;
		else if(!msgsent)	//Nothing could be sent or read, farmer cannot do useful work, so yield the process
			sleep(0);
	}
	//Stop loop when all messages, including termination messages have been sent

	//Wait until all hashes have been decrypted
	while(receivedMessageCount < MD5_LIST_NROF)
	{
		if(ReadMessage(mq_fd_response))
			receivedMessageCount++;
		else
			sleep(0); //If there are no messages from workers, yield the proces to allow workers to continue
	}
}

bool ReadMessage(mqd_t mq_fd_response)
{
	if(mq_receive(mq_fd_response, (char *) &rsp, sizeof(rsp), NULL) < 0) //If there is no message in the queue
	{
		return false;
	}
	int i;

	//Find the correct index to save the original string
	for(i = 0; i < MD5_LIST_NROF; i++)
	{
		if(rsp.hash == md5_list[i])
			break;
	}

	//Copy the original string from the message to the original string array
	original[i] = (char *) malloc(sizeof(char)*(MAX_MESSAGE_LENGTH+1));
	sprintf(original[i],"%s",rsp.string);
	return true;
}

bool SendMessage(mqd_t mq_fd_request)
{
	if(mq_send(mq_fd_request, (char *) &req, sizeof(req), 0) < 0) //If no message can be sent
	{
		return false;
	}
	return true;
}

void ConstructMessage(int hashindex, char startletter)
{
	req.terminate = 0;
	req.hash = md5_list[hashindex];
	req.letter = startletter;
	req.start_letter = ALPHABET_START_CHAR;
	req.last_letter = ALPHABET_END_CHAR;
}

void CreateWorkers(char mq_name1[], char mq_name2[])
{
	pid_t processID;
	for(int i = 0; i < NROF_WORKERS; i++)
	{
		processID = fork();
		if(processID < 0)
		{
			perror("fork() failed");
			exit(1);
		}
		else
		{
			if(processID == 0) //Child process
			{
				execlp("./worker","worker",mq_name1,mq_name2,NULL); //Load worker process with appropriate message queue names
				perror("execlp() failed");
				exit(1);
			}
			//Parent process continues
		}
	}
}

    // TODO:
    //  * create the message queues (see message_queue_test() in interprocess_basic.c)
    //  * create the child processes (see process_test() and message_queue_test())
    //  * do the farming
    //  * wait until the chilren have been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // Important notice: make sure that the names of the message queues contain your
    // student name and the process id (to ensure uniqueness during testing)



