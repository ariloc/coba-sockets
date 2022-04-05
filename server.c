#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 1 // TODO: PONER EN .h!

// localhost por defecto (?)
// ./srvFtp [PORT]
int main (int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "./srvFtp [PORT]\n");
        return 1;
    }

    struct addrinfo hints, *servinfo, *p;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err;
    if ((err = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        return 1;
    }

    int sockfd, yes = 1;
    for(p = servinfo; p; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            return 1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (!p) {
        fprintf(stderr, "client: failed to bind\n");
        return 2;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }

    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int new_fd;
    while(1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        if(send(new_fd, "220 srvFtp", 10, 0) == -1) 
            perror("send");
        //else fprintf(stderr, "OKA\n");

        close(new_fd);
    }

    close(sockfd); // TODO: (?)

    return 0;
}