.PHONY: all clean kv_test

CC=gcc
CPP=g++

OPTS =
DEBUG =
COPTS = -Wall -Wno-deprecated

CFLAGS = -c $(COPTS) ${OPTS} ${DEBUG} -D_LARGEFILE_SOURCE \
	-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_API -D_LARGEFILE64_SOURCE \
	-D_GNU_SOURCE -D__STDC_FORMAT_MACROS -fstrict-aliasing -Wall
CPPFLAGS = -c -fPIC -std=c++11 $(COPTS) ${OPTS} ${DEBUG} -D_LARGEFILE_SOURCE \
	  -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_API -D_LARGEFILE64_SOURCE \
	  -D_GNU_SOURCE -D__STDC_FORMAT_MACROS -fstrict-aliasing -fpermissive


LIBS = -lpthread -levent -lglog -lprotobuf -I/usr/local/lib/ -lcityhash -L/usr/local/lib/

dbg: OPTS += -O0
dbg: DEBUG += -g -ggdb
dbg: all

opti: OPTS += -O2
opti: all kvstorelib.o cuckoo_kvstore.o

all: kvstorelib.so kvstorelib.o cuckoo_kvstore.o

KVSTORELIB_OBJS = kvstorelib.o cuckoo_kvstore.o



stripall: $(TARGET)
	strip $(TARGET)E


kvstorelib.so: kvstorelib.o cuckoo_kvstore.o
	gcc kvstorelib.o cuckoo_kvstore.o -shared -o kvstorelib.so

kvstorelib.o: kvstorelib.cpp
	$(CPP) $(CPPFLAGS) $<

cuckoo_kvstore.o: cuckoo_kvstore.cpp
	$(CPP) $(CPPFLAGS) $<

clean:
	rm -rf *.o *.a $(TARGET)
