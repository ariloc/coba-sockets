#ifndef __USER_H__

#define __USER_H__

#include <stddef.h>

typedef struct _User {
    char *name, *pass;
} User;

/*
 * Creates a new User struct from fields.
 */
User* new_user (char *name, char *pass);

/*
 * Frees memory for a User (receives pointer).
 */
void free_user (User *x);

/*
 * Checks if two users are the same one by comparing the fields one by one.
 */
int user_equal (User *a, User *b);

#endif // __USER_H__