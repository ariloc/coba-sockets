#ifndef __CONNECTION_H__

#define __CONNECTION_H__

#include <stddef.h>
#include <stdio.h>

#define MAXSTRLEN 500
#define MAXDATASIZE 500

/*
 * Returns a socket given an ip and a port.
 */
int setup_socket(char *ip, char *port);

#endif // __CONNECTION_H__