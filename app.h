#ifndef APP_H
#define APP_H
#include "qtree.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// #include <time.h>
// #include <sys/time.h>

#define BUFSIZE 128
#define BACKLOG 5
#define INPUT_ARG_MAX_NUM 3
#define DELIM " \r\n"

// Use a linked list to maintain client data
typedef struct client {
	// File descriptor to write into and to read from
	int fd;
	// An array of int for client answers
	int *answers;
	// Before user entered a name, he cannot issue commands
	short state;
	// At most 128 char including the terminator '\0'
	char *usrname;
	// Designated buffer for user input
	char *buf;
	// Pointer to the current end-of-buf position
	int inbuf;
	// Pointer to the next client node
	struct client *next;
} Client;

// Utility methods
void error(char *msg);
int validate_user(char *name);
char *alloc_str(int size);
Client *addClient(int fd, char *name);
void removeClient(Client *cl);
int net_newline_location(char *buf, int inbuf);
int process_args(int, char **, QNode **, Node *, struct client *, struct client *);
int tokenize(char *, char **);
// Server methods
void bindAndListen(int port);
int acceptConn(int fd);

#endif
