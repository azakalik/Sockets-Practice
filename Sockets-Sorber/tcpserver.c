#include "common.h"
#include "tcpserver.h"


//server_socket is the passive socket id (to listen for new requests)
int server_socket = NOT_ALLOCATED;

typedef struct {
    int client_socket;
    char* client_address;
} ThreadData;

int main(int argc, char ** argv){
    SA_IN servaddr;
    uint8_t buff[MAXLINE+1];
    uint8_t recvline[MAXLINE+1];

    //Ctrl + C handling
    signal(SIGINT, handle_interrupt);

    //socket creation
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("Socket creation error");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //htonl converts 32bit host byte order to network byte order (endianness)
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //htonl converts 16bit host byte order to network byte order (endianness)
    servaddr.sin_port = htons(SERVER_PORT);

    // This avoids problems when restarting the server (if not it says 'address already in use' for some time)
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        err_n_die("setsockopt error");

    // === Bind and Listen make a socket listen in a specific port ===
    // server_socket is the passive socket we use to wait for new requests
    if ( bind(server_socket, (SA *) &servaddr, sizeof(servaddr)) < 0 )
        err_n_die("Bind error");

    if ((listen(server_socket, BACKLOG)) < 0)
        err_n_die("Listen error");
    // ===============================================================

    while(1){ //todo maxclients
        SA_IN addr;
        socklen_t addr_len = sizeof(addr);
        char client_address[MAXLINE+1];
        int client_socket;

        printf("Waiting for a connection on port %d\n", SERVER_PORT);
        fflush(stdout);

        // Accept blocks until it gets a connection
        // the last two arguments get the address of the one connecting
        // client_socket is the active socket we will be using to talk to the client
        if((client_socket = accept(server_socket, (SA*)&addr, &addr_len)) < 0)
            err_n_die("Client socket creation error");

        // Converts address in 'Network' format to 'Presentation' format
        inet_ntop(AF_INET, &(addr.sin_addr), client_address, MAXLINE);
        printf("Client connected. IP: %s\n", client_address);

        // Create a thread that handles the connection
        pthread_t thread;
        ThreadData * thread_data = malloc(sizeof(ThreadData));
        thread_data->client_socket = client_socket;
        thread_data->client_address = strdup(client_address);
        pthread_create(&thread, NULL, handle_connection, thread_data);
    }

}   

void handle_interrupt(int signal){
    if (signal == SIGINT){
        puts("");
        puts("Received Ctrl+C signal.");
        puts("Closing the sockets and exiting...");

        //Close the passive socket
        if(server_socket != NOT_ALLOCATED && close(server_socket) < 0)
            err_n_die("Close error while attempting to close passive socket");

        //TODO
        //Close the active socket
        // if(client_socket != NOT_ALLOCATED && close(client_socket) < 0)
        //     err_n_die("Close error while attempting to close active socket");

        puts("Exited succesfully.");
        exit(0);
    }
}

void * handle_connection(void * p_thread_data){
    ThreadData thread_data = *((ThreadData *) p_thread_data);
    int client_socket = thread_data.client_socket;
    char * client_address = thread_data.client_address;
    free(p_thread_data);

    char buffer[BUFSIZE];
    char filebuffer[BUFSIZE];
    char responsebuffer[BUFSIZE*2];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[PATHMAX+1];

    // First prints the hex (for debugging) and then prints the client's request
    while ( (bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer) - msgsize - 1) ) > 0){
        msgsize += bytes_read;

        //hacky way to detect end of message
        //it only checks for \n but it should check for \r\n\r\n
        if (msgsize > BUFSIZE - 1 || buffer[msgsize-1] == '\n') break;

    }

    if (bytes_read < 0)
        err_n_die("Read error");

    buffer[msgsize-1] = 0; //null terminate and remove \n

    printf("REQUEST (socket %d): %s\n", client_socket, buffer);
    fflush(stdout);

    FILE *fp = NULL;

    //path validity check
    if (realpath(buffer, actualpath) == NULL){
        // printf("ERROR(bad path): %s\n", buffer);
        // close(client_socket);
        // return NULL;
    } else {
        //read file
        fp = fopen(actualpath, "r");
        if (fp == NULL){
            printf("ERROR(open): %s\n", buffer);
            close(client_socket);
            return NULL;
        }
    }

    

    //create a response
    snprintf((char*)responsebuffer, sizeof(responsebuffer), "HTTP/1.0 200 OK\r\n\r\n<h1>Hello, user!!!</h1><p>Your IP is %s</p><p>If you requested a file, it is below</p>", client_address);
    free(client_address);

    //send the response to the client
    if(write(client_socket, (char*)responsebuffer, strlen((char*)responsebuffer)) < 0)
        err_n_die("Write error");

    if(fp != NULL){
        //send the file to the client
        //todo error check in fread
        while ((bytes_read = fread(filebuffer, 1, BUFSIZE, fp)) > 0){
            printf("Sending %zu bytes to client %d\n", bytes_read, client_socket);
            write(client_socket, filebuffer, bytes_read);
        }

        if(fclose(fp) < 0)
            err_n_die("Close error");
    }
    

    if(close(client_socket) < 0)
        err_n_die("Close error");

    printf("Closed connection with client %d\n", client_socket);
    return NULL;
}