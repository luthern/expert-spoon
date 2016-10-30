CC=gcc
CPP=g++
MAKE = make -f
CP = cp
MV = mv
AR = ar
RM = rm -f
RANLIB = ranlib
TOUCH = touch
INC =

DEBUG = 
#OPTS = -Wno-literal-suffix
OPTS = -Wno-literal-suffix -Wall
LDFLAGS = -lpthread 
CFLAGS = -c $(INC) $(OPTS) $(DEBUG) -D_LARGEFILE_SOURCE \
	-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_API -D_LARGEFILE64_SOURCE \
	-D_GNU_SOURCE -D__STDC_FORMAT_MACROS -fstrict-aliasing -Wall 
CPPFLAGS = -c -std=c++11 $(INC) $(OPTS) $(DEBUG) -D_LARGEFILE_SOURCE \
	-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_API -D_LARGEFILE64_SOURCE \
	-D_GNU_SOURCE -D__STDC_FORMAT_MACROS -fstrict-aliasing
	
OBJS = fde.o link.o msg.o net.o server.o resp.o
TARGET = client
UTIL_OBJS = ./util/log.o ./util/config.o ./util/bytes.o ./util/sorted_set.o ./util/app.o
UTIL_LIB = ./util/libutil.a

dbg: OPTS += -O0
dbg: LDFLAGS +=
#dbg: DEBUG += -D_UTEST_ -g -ggdb
dbg: DEBUG += -g -ggdb
dbg: all

opti: OPTS += -O2
opti: LDFLAGS += 
#opti: CPPFLAGS += -Wall
opti: all stripall

all: $(OBJS) $(UTIL_LIB) $(TARGET) 

stripall: $(TARGET)
	strip $(TARGET)
	mv -t bin/ $(TARGET) 

$(UTIL_LIB) : $(UTIL_OBJS)
	$(RM) $(UTIL_LIB)        
	$(AR) cq $(UTIL_LIB) $(UTIL_OBJS)      
	#$(RANLIB) $(UTIL_LIB) 

./util/app.o: ./util/app.cpp ./util/app.h
	${CPP} ${CPPFLAGS} -c -o $@ $<

./util/log.o: ./util/log.cpp ./util/log.h
	${CPP} ${CPPFLAGS} -c -o $@ $<

./util/config.o: ./util/config.cpp ./util/config.h
	${CPP} ${CPPFLAGS} -c -o $@ $<

./util/bytes.o: ./util/bytes.cpp ./util/bytes.h link.h
	${CPP} ${CPPFLAGS} -c -o $@ $<

./util/sorted_set.o: ./util/sorted_set.cpp ./util/sorted_set.h
	${CPP} ${CPPFLAGS} -c -o $@ $<

client: $(OBJS) $(UTIL_LIB)
	$(CPP)  $(OBJS) -o $@ $(UTIL_LIB) $(LDFLAGS)

fde.o: fde.cpp fde.h fde_epoll.cpp fde_select.cpp
	$(CPP) $(CPPFLAGS) $<

link.o: link.cpp link.h msg.h
	$(CPP) $(CPPFLAGS) $<

msg.o: msg.cpp link.h server.h
	$(CPP) $(CPPFLAGS) $< 

net.o: net.cpp net.h server.h
	$(CPP) $(CPPFLAGS) $< 

server.o: server.cpp net.h
	$(CPP) $(CPPFLAGS) $<

resp.o: resp.cpp resp.h
	$(CPP) $(CPPFLAGS) $<

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
	rm -fr *.o *.a $(TARGET) ./util/*.o ./util/*.a
