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

char buf[MAXDATASIZE];

typedef struct _SNode {
    void *data;
    struct _SNode* next;
} SNode;
typedef SNode* SList;

SList slist_new() {
    return NULL;
}

SList slist_push_front(SList list, void *data) {
    SList newItem = malloc(sizeof(SNode));
    newItem->data = data;
    newItem->next = list;

    return newItem;
}

typedef struct _User {
    char *name, *pass;
} User;

User* new_user (char *name, char *pass) {
    User *ret = malloc(sizeof(User));
    ret->name = name;
    ret->pass = pass;

    return ret;
}

// localhost por defecto (?)
// ./srvFtp [PORT]

char *parse_client_code(char *text){
    char *code;
    code = malloc(sizeof(char)*5);
    strncpy(code, text, 4);
    code[4] = '\0';
    return code;
}

SList load_ftpusers() {
    FILE *in = fopen("ftpusers","r");
    if (!in) {
        fprintf(stderr,"There was an error reading the ftpusers file.\n");
        return NULL;
    }

    SList ret = slist_new();
    while (fgets(buf,sizeof(buf),in)) {
        char *name, *pass;

        int pos_separator;
        for (pos_separator = 0; buf[pos_separator] && buf[pos_separator] != ':'; pos_separator++);
            
        if (buf[pos_separator] != ':') {
            fprintf(stderr,"Error parsing ftpusers file.\n");
            return NULL;
        }

        name = malloc(sizeof(char)*(pos_separator+1));
        // notar que buf incluye \n al final, entonces lo descontamos pero dejamos espacio para el terminador
        pass = malloc(sizeof(char)*(strlen(buf) - pos_separator - 1));

        strncpy(name,buf,pos_separator+1);
        name[pos_separator] = '\0';
        strcpy(pass,buf+pos_separator+1);
        pass[strlen(buf) - pos_separator - 2] = '\0';

        ret = slist_push_front(ret,new_user(name,pass));
    }

    return ret;
}

int main (int argc, char *argv[]) {
    
    if (argc != 2) {
        fprintf(stderr, "./srvFtp [PORT]\n");
        return 1;
    }

    SList db = load_ftpusers();
    if (!db) {
        fprintf(stderr,"No users loaded!\n");
        return 1;
    }

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
    int new_fd;
    char user[50], pass[50];

    while(1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        fprintf(stderr,"-- CONNECTION OPEN --\n");

        if(send(new_fd, "220 srvFtp", 10, 0) == -1) 
            perror("send");

        //Recibe nombre de usuario
        fprintf(stderr,"Connected successfully.\n");
        if(numbytes = recv(new_fd, buf, MAXDATASIZE, 0) == -1){
            perror("recv");
            return -1;
        }
        //Verifica que lo recibido sea un USER
        if(strcmp(parse_client_code(buf), "USER") == 0){
            char *user_pos_buf = buf+5;
            strcpy(user, user_pos_buf);
            user[strlen(user)-1] = '\0';
            fprintf(stderr,"User %s wants to log in.\n",user);
        }
        else{
            fprintf(stderr, "Didn't receive a username.\n");
            return 0;
        }
        //Envia solicitud de contraseña al cliente
        strcat(strcpy(buf, "331 Password required for "), user);
        if((send(new_fd, buf, strlen(buf)+1, 0)) == -1){
            perror("send");
            return -1;
        }
        //Recibir contraseña
        if(numbytes = recv(new_fd, buf, MAXDATASIZE, 0) == -1){
            perror("recv");
            return -1;
        }
        //Verifica que lo recibido sea una contraseña
        if(strcmp(parse_client_code(buf), "PASS") == 0){
            char *pass_pos_buf = buf+5;
            strcpy(pass, pass_pos_buf);
            pass[strlen(pass)-1] = '\0';
        }
        else{
            fprintf(stderr, "Didn't receive a password for user %s.\n",user);
            return 0;
        }
        //Validar usuario y contraseña

        SList aux;
        int exit = 1;
        for (aux = db; aux && exit; aux = aux->next) {
            if (!strcmp(user, ((User*) aux->data)->name) && !strcmp(pass, ((User*) aux->data)->pass)) {
                //login successful
                strcat(strcat(strcpy(buf, "230 User "), user), " logged in");
                fprintf(stderr,"User %s successfully logged in.\n",user);
                if(send(new_fd, buf, strlen(buf)+1, 0) == -1){
                    perror("send");
                    return -1;
                }
                exit = 0;
            }
        }

        if (exit) {
            //login incorrect
            fprintf(stderr,"Authentication error for user %s.\n",user);
            if(send(new_fd, "530 Login incorrect", 20, 0) == -1){
                perror("send");
                return -1;
            }
        }

        // connection would continue here otherwise...

        close(new_fd);
        fprintf(stderr,"-- CONNECTION CLOSED --\n\n");
    }

    close(sockfd); // TODO: (?)

    return 0;
}