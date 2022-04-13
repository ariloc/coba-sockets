#include <stdlib.h>
#include <string.h>

#include "user.h"

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