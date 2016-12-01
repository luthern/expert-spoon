TARGETS = kvstore
CC=g++ -g -O3 -Wall -Werror -fgnu89-inline
DPDK=1
PS=0
NETMAP=0

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


PS_DIR = /home/cs5204dpdk/mtcp/io_engine/
PS_INC = ${PS_DIR}/include
INC = -I./include/ ${UTIL_INC} ${MTCP_INC} -I${UTIL_FLD}/include -I../kvstorelib 
LIBS = ${MTCP_LIB} -I/usr/local/lib/ -lcityhash -L/usr/local/lib/ -L../kvstorelib ../kvstorelib/kvstorelib.o ../kvstorelib/cuckoo_kvstore.o
ifeq ($(PS),1)
INC += -I{PS_INC}
LIBS += -lmtcp -L${PS_DIR}/lib -lps -lpthread -lnuma -lrt
endif

ifeq ($(NETMAP),1)
LIBS += -lmtcp -lpthread -lnuma -lrt
endif

# CFLAGS for DPDK-related compilation
INC += ${MTCP_INC}
ifeq ($(DPDK),1)
DPDK_MACHINE_FLAGS = $(shell cat /home/cs5204dpdk/mtcp/dpdk/include/cflags.txt)
INC += ${DPDK_MACHINE_FLAGS} -I${DPDK_INC} -include $(DPDK_INC)/rte_config.h
endif

ifeq ($(DPDK),1)
DPDK_LIB_FLAGS = $(shell cat /home/cs5204dpdk/mtcp/dpdk/lib/ldflags.txt)
#LIBS += -m64 -g -O3 -pthread -lrt -march=native -Wl,-export-dynamic ${MTCP_FLD}/lib/libmtcp.a -L../../dpdk/lib -Wl,-lnuma -Wl,-lmtcp -Wl,-lpthread -Wl,-lrt -Wl,-ldl -Wl,${DPDK_LIB_FLAGS}
LIBS += -m64 -g -O3 -pthread -lrt -march=native -export-dynamic ${MTCP_FLD}/lib/libmtcp.a -L/home/cs5204dpdk/mtcp/dpdk/lib -lnuma -lmtcp -lpthread -lrt -ldl ${DPDK_LIB_FLAGS}
else
#LIBS += -m64 -g -O3 -pthread -lrt -march=native -Wl,-export-dynamic ${MTCP_FLD}/lib/libmtcp.a -L../../dpdk/lib -Wl,-lnuma -Wl,-lmtcp -Wl,-lpthread -Wl,-lrt -Wl,-ldl -Wl,${DPDK_LIB_FLAGS}
LIBS += -m64 -g -O3 -pthread -lrt -march=native -export-dynamic ${MTCP_FLD}/lib/libmtcp.a -L/home/cs5204dpdk/mtcp/dpdk/lib -lnuma -lmtcp -lpthread -lrt -ldl ${DPDK_LIB_FLAGS}
endif

all: kvstore

kvstore.o: kvstore.c
	${CC} -c $< ${CFLAGS} ${INC}

kvstore: kvstore.o
	${CC} $< ${LIBS} -o $@

clean:
	rm -f *~ *.o ${TARGETS} log_*

distclean: clean
	rm -rf Makefile
