#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <stdint.h>

#include "messages.h"

// Helper functions for printing message structures
void print_response_message(struct response_message * resp);
void print_send_message(struct send_message * send);

// Helper functions for messing with timespec structs
struct timespec* min_timespec(struct timespec *t1, struct timespec *t2);
struct timespec* max_timespec(struct timespec *t1, struct timespec *t2);
void copy_timespec(struct timespec *dest, struct timespec *src);

// Function that simplifies the socket connection process
int init_connection(uint32_t ip_addr, uint16_t port);
#endif /* UTILS_H */
