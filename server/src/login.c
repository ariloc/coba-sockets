#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "login.h"
#include "connection.h"

User* login_user (int sock, SList db, char* comm) { // NULL in case of any error, a POINTER to the user in db if login succeeded
    char buf[MAXDATASIZE];
    int numbytes;
    User *request = new_user(NULL,NULL);

    char *name = malloc(sizeof(char)*(strlen(comm) - 5));
    request->name = strcpy(name, comm+5);
    fprintf(stderr,"User %s wants to log in.\n",request->name);

    //Envia solicitud de contraseña al cliente
    strcat(strcpy(buf, "331 Password required for "), request->name);
    if((send(sock, buf, strlen(buf), 0)) == -1){
        perror("send");
        goto end_login;
    }

    //Recibir contraseña
    if((numbytes = recv(sock, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        goto end_login;
    }
    buf[numbytes] = '\0';

    //Verifica que lo recibido sea una contraseña
    if(matches_client_code(buf, "PASS")){
        char *pass = malloc(sizeof(char)*(numbytes-5));
        request->pass = strcpy(pass, buf+5);
    }
    else{
        fprintf(stderr, "Didn't receive a password for user %s.\n",request->name);
        goto end_login;
    }

    //Validar usuario y contraseña
    SList db_user;
    for (db_user = db; db_user && !user_equal(request,db_user->data); db_user = db_user->next);
    if (db_user) {
        //login successful
        strcat(strcat(strcpy(buf, "230 User "), request->name), " logged in");
        fprintf(stderr,"User %s successfully logged in.\n",request->name);

        if(send(sock, buf, strlen(buf), 0) == -1){
            perror("send");
            goto end_login;
        }

        return db_user->data;
    }

    //login incorrect
    fprintf(stderr,"Authentication error for user %s.\n",request->name);
    if(send(sock, "530 Login incorrect", 20, 0) == -1)
        perror("send");

// Uso relevante de goto, para evitar memory leaks y manejar errores a la vez.
end_login:
    free_user(request);
    return NULL;
}