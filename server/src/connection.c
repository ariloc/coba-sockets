#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "connection.h"
#include "login.h"

int matches_client_code (char *text, const char* code) {
    return !strncmp(text,code,4);
}

int setup_socket(char *port) { // returns sockfd
    struct addrinfo hints, *servinfo, *p;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err;
    if ((err = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        return -1;
    }

    int sockfd, yes = 1;
    for(p = servinfo; p; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            return -1;
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
        return -1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return -1;
    }

    return sockfd;
}

int accept_connections (int sock, SList db) {
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int new_fd;

    while(1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sock, (struct sockaddr*)&their_addr, &sin_size);
        if (new_fd == -1) {
            fprintf(stderr,"%d\n",sock);
            perror("accept");
            continue;
        }
        fprintf(stderr,"-- CONNECTION OPEN --\n");

        if(send(new_fd, "220 srvFtp", 10, 0) == -1) {
            perror("send");
            goto end_connection;
        }
        fprintf(stderr,"Connected successfully.\n");

        //Recibe comando
        while (recv_command(new_fd,db) != -1);

    // en caso de que haya errores, siempre cerrar new_fd
    end_connection: 
        close(new_fd);
        fprintf(stderr,"-- CONNECTION CLOSED --\n\n");
    }

    close(sock);
    return 1;
}

int recv_command (int sock, SList db) { // TODO: Se le pasaria un User* luego para manejar cosas referidas a ese usuario.
    char buf[MAXDATASIZE];

    int numbytes;
    if((numbytes = recv(sock, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        goto end_recv_fail;
    }
    buf[numbytes] = '\0';

    char *comm = malloc(sizeof(char)*(numbytes+1));
    strcpy(comm, buf);

    // Verifica que lo recibido sea un USER
    if(matches_client_code(buf, "USER")){
        if (login_user(sock, db, comm) == NULL) goto end_recv_fail;
        goto end_recv_success;
    }

end_recv_fail:
    return -1;

end_recv_success:
    return 1;
}