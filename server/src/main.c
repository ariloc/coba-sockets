#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "slist.h"
#include "user.h"
#include "load_users.h"
#include "connection.h"
#include "login.h"

// localhost por defecto (?)
// ./srvFtp [PORT]

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

    int err_code;
    if ((err_code = accept_connections(sockfd,db)) == -1) {
        return 1;
    }

    return 0;
}