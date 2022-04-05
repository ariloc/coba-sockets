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
#define MAXDATASIZE 500 



// localhost por defecto (?)
// ./srvFtp [PORT]

char *parse_client_code(char *text){
    char *code;
    code = malloc(sizeof(char)*5);
    strncpy(code, text, 4);
    code[4] = '\0';
    return code;
}

int main (int argc, char *argv[]) {
    
    if (argc != 2) {
        fprintf(stderr, "./srvFtp [PORT]\n");
        return 1;
    }

    char *buf;
    buf = malloc(sizeof(char)*MAXDATASIZE);
    int numbytes;
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
    int new_fd, exit;
    char user[50], pass[50];

    while(1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        if(send(new_fd, "220 srvFtp", 10, 0) == -1) 
            perror("send");
        else fprintf(stderr, "OKA\n");
        exit = 1;
        while(exit){
            //Recibe nombre de usuario
            printf("\nComienzo Login\n");
            memset(buf, 0, sizeof(buf));    
            if(numbytes = recv(new_fd, buf, MAXDATASIZE, 0) == -1){
                perror("recv");
                return -1;
            }
            //Verifica que lo recibido sea un USER
            if(strcmp(parse_client_code(buf), "USER") == 0){
                buf = &buf[5];
                strcpy(user, buf);
                user[strlen(user)-1] = '\0';
            }
            else{
                fprintf(stderr, "Didn't recieve a username.");
                return 0;
            }
            //Envia solicitud de contraseña al cliente
            memset(buf, 0, sizeof(buf));
            strcat(strcpy(buf, "331 Password required for "), user);
            if((send(new_fd, buf, strlen(buf), 0)) == -1){
                perror("send");
                return -1;
            }
            //Recibir contraseña
            memset(buf, 0, sizeof(buf));
            if(numbytes = recv(new_fd, buf, MAXDATASIZE, 0) == -1){
                perror("recv");
                return -1;
            }
            //Verifica que lo recibido sea una contraseña
            if(strcmp(parse_client_code(buf), "PASS") == 0){
                buf = &buf[5];
                strcpy(pass, buf);
                pass[strlen(pass)-1] = '\0';
            }
            else{
                fprintf(stderr, "Didn't recieve a password.");
                return 0;
            }
            //Validar usuario y contraseña
            if(strcmp("manu", user) == 0 && strcmp("1234", pass) == 0){
                //login successful
                strcat(strcat(strcpy(buf, "230 User "), user), " logged in\n");
                if(send(new_fd, buf, strlen(buf), 0) == -1){
                    perror("send");
                    return -1;
                }
                exit = 0;
            }
            else{
                //login incorrect
                if(send(new_fd, "530 Login incorrect\n", 20, 0) == -1){
                    perror("send");
                    return -1;
                }
            }
        }

        close(new_fd);
    }

    close(sockfd); // TODO: (?)

    return 0;
}