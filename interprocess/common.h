/* 
 * Operating Systems {2INCO} Practical Assignment
 * Interprocess Communication
 *
 * Contains definitions which are commonly used by the farmer and the workers
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include "uint128.h"

// maximum size for any message in the tests
#define MAX_MESSAGE_LENGTH	6
 

typedef struct {
		int							terminate;
		uint128_t				hash;
		char 						letter;
		char						start_letter;
		char						last_letter;
} MQ_REQUEST_MESSAGE;

typedef struct
{
    uint128_t				hash;
    char 						string[MAX_MESSAGE_LENGTH+1];
} MQ_RESPONSE_MESSAGE;

#endif

