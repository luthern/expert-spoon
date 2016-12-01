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

int8_t kvstore_process_packet(char * pkt_buf)
{
	//Handle processing code here. Invoke cuckoo hash functions from here
	bool result = false;
	struct send_message *send = (struct send_message *)(pkt_buf);
	printf("OPCODE: %d\n %d\n", send->operation, GET);
	switch(send->operation)
	{
		case GET:
			//TRACE_APP("Received a GET request for %u\n", send->key);
			printf("Received a GET request\n");
			result = (bool) ptr_KVStore->find(KEY_CAST(send->key),&VALUE_CAST(send->value)); 			
			if (result) { 
				pkt_buf[0] = OK;
			}
			else {
				pkt_buf[0] = ERROR;
			} 
			break;
		case PUT:
			//TRACE_APP("Received a PUT request for %u,%u\n", send->key, send->value);
			printf("Received a PUT request\n");
			result = (bool) ptr_KVStore->insert(KEY_CAST(send->key), VALUE_CAST(send->value));
			if (result) { 
				pkt_buf[0] = OK;
			}
			else {
				pkt_buf[0] = ERROR;
			}
			break;
		case DELETE:
			//TRACE_APP("Received a DELETE request for %u\n", send->key);
			printf("Received a DELETE request\n");
			result = (bool) ptr_KVStore->erase(KEY_CAST(send->key));
			if (result) { 
				pkt_buf[0] = OK;
			}
			else {
				pkt_buf[0] = ERROR;
			} 			
			break;
		case UPDATE:
			printf("Received an UPDATE request\n");
			result = (bool) ptr_KVStore->update(KEY_CAST(send->key), VALUE_CAST(send->value));
			if (result) { 
				pkt_buf[0] = OK;
			}
			else {
				pkt_buf[0] = ERROR;
			} 
		default:
			//TRACE_ERROR("Bad KVSTORE request opcode\n");
			printf("Received a bad KVSTORE request opcode %d\n", send->operation);
			exit(1);
	}
	printf("\n kvstore_process_packet returns %i ",result);
	return result;
}

#ifdef __cplusplus
};
#endif

