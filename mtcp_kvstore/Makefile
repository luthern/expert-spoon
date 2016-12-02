TARGETS = kvstore
CC=gcc -g -O3 -Wall -fgnu89-inline
DPDK=1

# DPDK LIBRARY and HEADER
DPDK_INC=/home/cs5204dpdk/mtcp/dpdk/include
DPDK_LIB=/home/cs5204dpdk/mtcp/dpdk/lib/

# mtcp library and header
MTCP_FLD    =/home/cs5204dpdk/mtcp/mtcp
MTCP_INC    =-I${MTCP_FLD}/include
MTCP_LIB    =-L${MTCP_FLD}/lib
MTCP_TARGET = ${MTCP_LIB}/libmtcp.a

UTIL_FLD = /home/cs5204dpdk/mtcp/util
UTIL_INC = -I${UTIL_FLD}/include

INC = -I./include/ ${UTIL_INC} ${MTCP_INC} -I${UTIL_FLD}/include -I../kvstorelib
LIBS = ${MTCP_LIB} -I/usr/local/lib/ -lcityhash -L/usr/local/lib/ -L../kvstorelib -lkvstorelib

# CFLAGS for DPDK-related compilation
INC += ${MTCP_INC}
DPDK_MACHINE_FLAGS = $(shell cat /home/cs5204dpdk/mtcp/dpdk/include/cflags.txt)
INC += ${DPDK_MACHINE_FLAGS} -I${DPDK_INC} -include $(DPDK_INC)/rte_config.h
DPDK_LIB_FLAGS = $(shell cat /home/cs5204dpdk/mtcp/dpdk/lib/ldflags.txt)
LIBS += -m64 -g -O3 -pthread -lrt -march=native -export-dynamic ${MTCP_FLD}/lib/libmtcp.a -L/home/cs5204dpdk/mtcp/dpdk/lib -lnuma -lmtcp -lpthread -lrt -ldl ${DPDK_LIB_FLAGS}

all: kvstore

kvstore.o: kvstore.c
	${CC} -c $< ${CFLAGS} ${INC}

kvstore: kvstore.o
	${CC} $< ${LIBS} -o $@

clean:
	rm -f *~ *.o ${TARGETS} log_*

distclean: clean
	rm -rf Makefile
