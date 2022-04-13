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
#include "login.h"
#include "auxiliar.h"


int login(int sock){
    char *nombre_usuario, *contrasena, buf[MAXDATASIZE];
    int numbytes;
    
    //solicitud de usuario "username: "
    printf("username: ");
    read_string_buf(buf);
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
    read_string_buf(buf),
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