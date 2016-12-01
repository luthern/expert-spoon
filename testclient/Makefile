CC=gcc
CPP=g++
MAKE = make -f
CP = cp
MV = mv
AR = ar
RM = rm -f
RANLIB = ranlib
TOUCH = touch
INC = -I../kvstorelib

DEBUG =
#OPTS = -Wno-literal-suffix
OPTS = -Wno-literal-suffix -Wall
LDFLAGS = -lrt
CFLAGS = -c $(INC) $(OPTS) $(DEBUG) -D_LARGEFILE_SOURCE \
	-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_API -D_LARGEFILE64_SOURCE \
	-D_GNU_SOURCE -D__STDC_FORMAT_MACROS -fstrict-aliasing -Wall
CPPFLAGS = -c -std=c++11 $(INC) $(OPTS) $(DEBUG) -D_LARGEFILE_SOURCE \
	-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_API -D_LARGEFILE64_SOURCE \
	-D_GNU_SOURCE -D__STDC_FORMAT_MACROS -fstrict-aliasing

OBJS = client.o utils.o
TARGET = client-5204

dbg: OPTS += -O0
dbg: LDFLAGS +=
#dbg: DEBUG += -D_UTEST_ -g -ggdb
dbg: DEBUG += -g -ggdb
dbg: all

opti: OPTS += -O2
opti: LDFLAGS +=
#opti: CPPFLAGS += -Wall
opti: all stripall

all: $(OBJS) $(TARGET)

stripall: $(TARGET)
	strip $(TARGET)
	mv -t bin/ $(TARGET)

client-5204: $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) -o $@

utils.o: utils.cpp utils.h
	$(CPP) $(CPPFLAGS) -o $@ $<

client.o: main.cpp
	$(CPP) $(CPPFLAGS) -o $@  $<

#valgrind:
#	G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --track-origins=yes --leak-resolution=high --leak-check=full --show-reachable=yes  ./unit_test

#cleanlog:
#	rm -fr /tmp/cachesim.* /tmp/unit_test.*
#test:
#	./unit_test
#
#cscope:
#	cscope -Rbkq

#clean:
#	rm -fr *.o *.a

clean:
	rm -fr *.o *.a $(TARGET)
