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

int8_t kvstore_process_packet(const char * pkt_buf, struct response_message * resp)
{
	//Handle processing code here. Invoke cuckoo hash functions from here
	bool result = false;
	//struct send_message *send = (struct send_message *)(pkt_buf);
	memcpy(resp, pkt_buf, 50);
	//printf("OPCODE: %d\n", send->operation);
 	switch(resp->status_code)
	{
		case GET:
			result = (bool) ptr_KVStore->find(KEY_CAST(resp->key),&VALUE_CAST(resp->value)); 			
			break;
		case PUT:
			result = (bool) ptr_KVStore->insert(KEY_CAST(resp->key), VALUE_CAST(resp->value));
			break;
		case DELETE:
			result = (bool) ptr_KVStore->erase(KEY_CAST(resp->key));
			break;
		case UPDATE:
			result = (bool) ptr_KVStore->update(KEY_CAST(resp->key), VALUE_CAST(resp->value));
			break;
		default:
			//for (int i = 0; i < 50; i++) {
			//	printf("%02x", pkt_buf[i]);
			//}
			//printf("\n");
			printf("Received a bad KVSTORE request opcode %d\n", resp->status_code);
			exit(1);
	}
	if (result) { 
		resp->status_code = OK;
	}
	else {
		resp->status_code = ERROR;
	}
	//printf("kvstore_process_packet returns %i\n",result);
	return result;
}

#ifdef __cplusplus
};
#endif

