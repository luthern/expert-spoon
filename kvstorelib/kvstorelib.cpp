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
	//printf("OPCODE: %d\n", send->operation);
	switch(send->operation)
	{
		case GET:
			result = (bool) ptr_KVStore->find(KEY_CAST(send->key),&VALUE_CAST(send->value)); 			
			break;
		case PUT:
			result = (bool) ptr_KVStore->insert(KEY_CAST(send->key), VALUE_CAST(send->value));
			break;
		case DELETE:
			result = (bool) ptr_KVStore->erase(KEY_CAST(send->key));
			break;
		case UPDATE:
			result = (bool) ptr_KVStore->update(KEY_CAST(send->key), VALUE_CAST(send->value));
			break;
		default:
			printf("Received a bad KVSTORE request opcode %d\n", send->operation);
			exit(1);
	}
	if (result) { 
		pkt_buf[0] = OK;
	}
	else {
		pkt_buf[0] = ERROR;
	}

	//printf("kvstore_process_packet returns %i\n",result);
	return result;
}

#ifdef __cplusplus
};
#endif

