/*
 * kvstorelib.cpp
 *
 *  Created on: Nov 30, 2016
 *      Author: ved
 */
#include "cuckoo_kvstore.h"
#include "kvstorelib.h"
#include "messages.h"
#include <stdio.h>

#ifdef __cplusplus

extern "C" {
#endif

static KVStore *ptr_KVStore = NULL;

int8_t kvstore_create()
{
	if (ptr_KVStore == NULL)
	{
		ptr_KVStore = new CuckooKVStore();
	}
	return 0;
}

int8_t kvstore_destroy()
{
	delete ptr_KVStore;
	return 0;
}

int8_t kvstore_process_packet(uint8_t * pkt_buf)
{
	//Handle processing code here. Invoke cuckoo hash functions from here
	int result= 0;
	send_message *send = (send_message *)(pkt_buf);
	printf("OPCODE: %d\n %d\n", send->operation, GET);
	switch(send->operation)
	{
		case GET:
			//TRACE_APP("Received a GET request for %u\n", send->key);
			printf("Received a GET request\n");
			break;
		case PUT:
			//TRACE_APP("Received a PUT request for %u,%u\n", send->key, send->value);
			printf("Received a PUT request\n");
			break;
		case DELETE:
			//TRACE_APP("Received a DELETE request for %u\n", send->key);
			printf("Received a DELETE request\n");
			break;
		case TERMINATE:
			// CloseConnection. May need more arguments for this function... :(
			//TRACE_APP("Received a TERMINATE request\n");
			printf("Received a TERMINATE request\n");
			break;
		default:
			//TRACE_ERROR("Bad KVSTORE request opcode\n");
			printf("Received a bad KVSTORE request opcode %d\n", send->operation);
			exit(1);
	}
	return result;
}

#ifdef __cplusplus
};
#endif

