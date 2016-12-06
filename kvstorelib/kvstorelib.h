/*
 * kvstorelib.h
 *
 *  Created on: Nov 30, 2016
 *      Author: ved
 */

#ifndef KVSTORELIB_H_
#define KVSTORELIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "messages.h"

int8_t kvstore_create();
int8_t kvstore_destroy();
int8_t kvstore_process_packet( const char * pkt_buf, struct response_message * resp);

#ifdef __cplusplus
}
#endif

#endif /* KVSTORELIB_H_ */
