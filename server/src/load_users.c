#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "load_users.h"

SList load_ftpusers() {
    char buf[MAXDATASIZE];

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