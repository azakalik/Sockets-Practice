#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include "common.h"
#include <pthread.h>

#define BUFSIZE 4096
#define PATHMAX 1024

// max connections that go to the queue (if there a more they fail)
#define BACKLOG 50

void handle_interrupt(int signal);
void * handle_connection(void * p_client_socket);

struct connection {
    int client_socket;
    char * client_address;
};

#endif