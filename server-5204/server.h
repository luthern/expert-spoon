#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>


#define CO_OK      0
#define CO_ERR    -1
#define CO_EAGAIN -2
#define CO_ENOMEM -3


typedef int rstatus_t;

static inline double millitime() {
	struct timeval now;
	gettimeofday(&now, NULL);
	double ret = now.tv_sec + now.tv_usec/1000.0/1000.0;

	return ret;
}

static inline int64_t time_ms() {
	struct timeval now;
	gettimeofday(&now, NULL);

	return now.tv_sec*1000 + now.tv_usec/1000;
}

#endif
