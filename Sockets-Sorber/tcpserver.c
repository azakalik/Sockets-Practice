#include "common.h"


//listenfd is the passive socket id (to listen for new requests)
int listenfd = NOT_ALLOCATED;

//connfd is the active socket id (to answer the requests)
int connfd = NOT_ALLOCATED;


void handle_interrupt(int signal){
    if (signal == SIGINT){
        puts("");
        puts("Received Ctrl+C signal.");
        puts("Closing the sockets and exiting...");

        //Close the passive socket
        if(listenfd != NOT_ALLOCATED && close(listenfd) < 0)
            err_n_die("Close error while attempting to close passive socket");

        //Close the active socket
        if(connfd != NOT_ALLOCATED && close(connfd) < 0)
            err_n_die("Close error while attempting to close active socket");

        puts("Exited succesfully.");
        exit(0);
    }
    return;
}

int main(int argc, char ** argv){
    struct sockaddr_in servaddr;
    uint8_t buff[MAXLINE+1];
    uint8_t recvline[MAXLINE+1];

    //Ctrl + C handling
    signal(SIGINT, handle_interrupt);

    //socket creation
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("Socket creation error");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //htonl convert 32bit host byte order to network byte order (endianness)
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //htonl convert 16bit host byte order to network byte order (endianness)
    servaddr.sin_port = htons(SERVER_PORT);

    // === Bind and Listen make a socket listen in a specific port ===
    // listenfd is the passive socket we use to wait for new requests
    if ( bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) < 0 )
        err_n_die("Bind error");

    if ((listen(listenfd, 10)) < 0)
        err_n_die("Listen error");
    // ===============================================================

    while(1){
        struct sockaddr_in addr;
        socklen_t addr_len;
        char client_address[MAXLINE+1];

        printf("Waiting for a connection on port %d\n", SERVER_PORT);
        fflush(stdout);

        // Accept blocks until it gets a connection
        // the last two arguments get the address of the one connecting
        // connfd is the active socket we will be using to talk to the client
        connfd = accept(listenfd, (SA*)&addr, &addr_len);

        // Converts address in 'Network' format to 'Presentation' format
        inet_ntop(AF_INET, &(addr.sin_addr), client_address, MAXLINE);
        printf("Client connected: %s\n", client_address);

        //clear the buffer
        memset(recvline, 0, MAXLINE);

        // First prints the hex (for debugging) and then prints the client's request
        int n;
        while ( (n = read(connfd, recvline, MAXLINE - 1) ) > 0){
            fprintf(stdout, "\n%s\n\n%s", bin2hex(recvline, n), recvline);
        
            //hacky way to detect end of message
            //it only checks for \r but it should check for \r\n\r\n
            if (recvline[n-1] == '\n')
                break;

            memset(recvline, 0, MAXLINE);
        }

        if (n< 0)
        err_n_die("Read error");

        //create a response
        snprintf((char*)buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\n<h1>Hello, user!!!</h1><p>Your IP is %s</p>", client_address);

        //send the response to the client
        if(write(connfd, (char*)buff, strlen((char*)buff)) < 0)
            err_n_die("Write error");

        if(close(connfd) < 0)
            err_n_die("Close error");

        connfd = NOT_ALLOCATED;
    }

}   