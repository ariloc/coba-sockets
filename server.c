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

void free_user (User *x) {
    free(x->name);
    free(x->pass);
    free(x);
}

int user_equal (User *a, User *b) {
    return !strcmp(a->name,b->name) && !strcmp(a->pass,b->pass);
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

        int n = strlen(buf);

        if (pos_separator < 1 || pos_separator >= n-1) {
            fprintf(stderr,"User and password length have to be non-zero.\n");
            return NULL;
        }

        if (buf[n-1] == '\n') buf[n--] = '\0';

        name = malloc(sizeof(char)*(pos_separator+1));
        // notar que buf incluye \n al final, entonces lo descontamos pero dejamos espacio para el terminador
        pass = malloc(sizeof(char)*(n - pos_separator));

        strncpy(name,buf,pos_separator+1);
        name[pos_separator] = '\0';
        strcpy(pass,buf+pos_separator+1);
        pass[n - pos_separator - 1] = '\0';

        ret = slist_push_front(ret,new_user(name,pass));
    }

    return ret;
}

User* login_user (int sock, SList db) { // NULL in case of any error, a POINTER to the user in db if login succeeded
    int numbytes;
    User *request = new_user(NULL,NULL);

    if(send(sock, "220 srvFtp", 10, 0) == -1) {
        perror("send");
        goto end_login;
    }

    //Recibe nombre de usuario
    fprintf(stderr,"Connected successfully.\n");
    if((numbytes = recv(sock, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        goto end_login;
    }
    buf[numbytes] = '\0';

    //Verifica que lo recibido sea un USER
    if(!strcmp(parse_client_code(buf), "USER")){
        char *name = malloc(sizeof(char)*(numbytes-5));
        request->name = strcpy(name, buf+5);
        fprintf(stderr,"User %s wants to log in.\n",request->name);
    }
    else{
        fprintf(stderr, "Didn't receive a username.\n");
        goto end_login;
    }

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
    if(strcmp(parse_client_code(buf), "PASS") == 0){
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

    int sockfd;
    if ((sockfd = setup_socket(argv[1])) == -1) {
        fprintf(stderr,"Shutting down...\n");
        return 1;
    }

    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int new_fd;

    while(1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if (new_fd == -1) {
            fprintf(stderr,"%d\n",sockfd);
            perror("accept");
            continue;
        }

        fprintf(stderr,"-- CONNECTION OPEN --\n");

        User *usr;
        if ((usr = login_user(new_fd,db)) == NULL)
            goto end_connection;

        // connection would continue here otherwise...

    end_connection:
        close(new_fd);
        fprintf(stderr,"-- CONNECTION CLOSED --\n\n");
    }

    close(sockfd); // TODO: (?)

    return 0;
}