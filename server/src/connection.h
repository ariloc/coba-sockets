#ifndef __CONNECTION_H__

#define __CONNECTION_H__

#include <stddef.h>
#include "slist.h"

#define MAXDATASIZE 500
#define BACKLOG 1

/*
 * Checks if the provided client code matches the received message from the client.
 */
int matches_client_code (char *text, const char* code);

/*
 * Returns a socket file descriptor given a port number.
 */
int setup_socket(char *port);

/*
 * Starts accepting connections from the "sock" file descriptor.
 * Also given as an argument, SList db of User*.
 */
int accept_connections (int sock, SList db);

/*
 * Starts receiving commands from a connected client and processing them.
 * Also given as an argument, SList db of User*.
 */
int recv_command (int sock, SList db);

#endif // __CONNECTION_H__