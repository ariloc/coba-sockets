#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "connection.h"
#include "login.h"
#include "auxiliar.h"

// ./clFtp [IP] [PORT]

#define MAXSTRLEN 500
#define MAXDATASIZE 500

int main (int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr,"./clFtp [IP] [PORT]\n");
        return 1;
    }
    
    int sockfd;

    if ((sockfd = setup_socket(argv[1], argv[2])) == -1) {
        perror("client: socket");
        return 1;
    }
    
    login(sockfd);

    close(sockfd);

    return 0;
}