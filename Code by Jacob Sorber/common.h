#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/socket.h> //basic socket definitions
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> //for inet_pton
#include <stdarg.h> //for variable argument functions
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

//the port I will be communicating to
//80 is the standard HTTP port
#define SERVER_PORT 8082

//buffer size
#define MAXLINE 4096

//for less verbose code
#define SA struct sockaddr

//to distinguish between allocated and not allocated socket ids
#define NOT_ALLOCATED -1

//for error handling
void err_n_die(const char * fmt, ...);
char * bin2hex(const unsigned char * input, size_t len);

#endif