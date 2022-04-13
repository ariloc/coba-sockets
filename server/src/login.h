#ifndef __LOGIN_H__

#define __LOGIN_H__

#include <stddef.h>
#include "user.h"
#include "slist.h"

#define MAXDATASIZE 500

/*
 * Given a socket file descriptor, the SList "db" of User*,
 * and the USER command received from the client, follows the
 * the login process and returns a User* if authentication succeeds.
 * Returns NULL otherwise.
 */
User* login_user (int sock, SList db, char* comm);

#endif // __LOGIN_H__