#ifndef __LOAD_USERS_H__

#define __LOAD_USERS_H__

#include <stddef.h>
#include "slist.h"

#define MAXDATASIZE 500

/*
 * Load users and passwords from "ftpusers" file,
 * returns SList with User* as data (void*).
 */
SList load_ftpusers();

#endif // __LOAD_USERS_H__