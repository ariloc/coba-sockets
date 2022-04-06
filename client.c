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

// ./clFtp [IP] [PORT]

#define MAXSTRLEN 500
#define MAXDATASIZE 500 

//TODO: Pasar al .h
char buf[MAXDATASIZE];

int parse_response_code(char *code){
    return 100*(code[0]-'0') + 10*(code[1]-'0') + (code[2]-'0');
}

void read_string_buf() {
    fgets(buf, MAXSTRLEN, stdin);
    buf[strcspn(buf, "\r\n")] = '\0';
}

int login(int sock){
    char *nombre_usuario, *contrasena;
    int numbytes;
    
    //solicitud de usuario "username: "
    printf("username: ");
    read_string_buf();
    nombre_usuario = malloc(sizeof(char)*6 + strlen(buf));
    strcat(strcpy(nombre_usuario, "USER "), buf);
    
    //Enviar nombre de usuario "USER <nombre_usuario>"
    if(send(sock, nombre_usuario, strlen(nombre_usuario), 0) == -1){
        perror("send");
        return -1;
    }

    //Recibir solicitud de contraseña "password required for <nombre_usuario> "
    if((numbytes = recv(sock, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        return -1;
    }
    buf[numbytes] = '\0';
    
    printf("%s\n", buf);

    if(parse_response_code(buf) != 331){
        perror("response code");
        return -1;
    }

    printf("password: ");
    read_string_buf(),
    contrasena = malloc(sizeof(char)*6 + strlen(buf));
    strcat(strcpy(contrasena, "PASS "), buf);

    //Enviar contraseña "PASS <contraseña>"
    if(send(sock, contrasena, strlen(contrasena), 0) == -1){
        perror("send");
        return -1;
    }

    //Recibir respuesta de user:pass
    if((numbytes = recv(sock, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        return -1;
    }
    buf[numbytes] = '\0';

    //Successful login
    if(parse_response_code(buf) == 230){
        printf("%s\n", buf);
        return 1;
    }
    //Login error
    if(parse_response_code(buf) == 530){
        printf("%s\n", buf);
        return 0;
    }
    //Unknown error
    fprintf(stderr, "Login failed, unknown error");
    return -1;
}

int main (int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr,"./clFtp [IP] [PORT]\n");
        return 0;
    }

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err;
    if ((err = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        return 1;
    }

    int sockfd, numbytes;
    for(p = servinfo; p; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (!p) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }


    freeaddrinfo(servinfo);

    
    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        return 1;
    }
    
    if(parse_response_code(buf) != 220){
        perror("response code");
        return 1;
    }
    buf[numbytes] = '\0';
    printf("%s\n",buf);
    login(sockfd);

    /*
    int numbytes;
    int MAXDATASIZE = 50;
    char buf[50];
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    buf[numbytes] = '\0';
    printf("%s\n",buf);
    */

    close(sockfd);

    return 0;
}