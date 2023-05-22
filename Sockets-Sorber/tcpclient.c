#include "common.h"


int main(int argc, char ** argv){
    int sockfd, n;
    int sendbytes;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    //check if the user is using correctly the program
    if (argc != 2){
        err_n_die("usage: %s <server address>", argv[0]);
    }

    //======= create a new socket ========
    //AF_INET: Address Family - Internet
    //SOCK_STREAM: Stream Socket (as opposed to Datagram Socket, which is an only package)
    //0: use the default for Stream Socket (TCP)
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("Error while creating the socket!");

    //clean the struct
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    //htons solves endianness problems using the network standard order
    servaddr.sin_port = htons(SERVER_PORT);

    //convert the IP address from char[] to binary
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        err_n_die("inet_pton error for %s ", argv[1]);

    //connect to the server
    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        err_n_die("Connection failed");

    //now we are connected. Prepare the message.
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
    sendbytes = strlen(sendline);

    //write the request into the socket
    //if at least one character doesn't send, the program exits. this can be improved
    if (send(sockfd, sendline, sendbytes, 0) != sendbytes)
        err_n_die("Send error");
    
    memset(recvline, 0, MAXLINE);
    //read the server's answer
    while( (n = recv(sockfd, recvline, MAXLINE-1, 0)) > 0 ){
        printf("%s", recvline);
        memset(recvline, 0, MAXLINE);
    }

    if(n < 0)
        err_n_die("Read error");

    exit(0);
}