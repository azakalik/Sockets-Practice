CC=gcc
CFLAGS= -g #this shows debug information
COMMON_OBJS = common.o
PROGRAMS = tcpc tcps

all: $(COMMON_OBJS) $(PROGRAMS)

tcpc: tcpclient.c $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o tcpc tcpclient.c $(COMMON_OBJS)

tcps: tcpserver.c $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o tcps tcpserver.c $(COMMON_OBJS)

clean:
	rm -rf *.dSYM $(PROGRAMS) *.o