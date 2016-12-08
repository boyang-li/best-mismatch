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

#define BUFSIZE 1024
#define NAME_SIZE 128
#define BACKLOG 5
#define INPUT_ARG_MAX_NUM 3
#define DELIM " \r\n"
#define PORT 5692

// Use a linked list to maintain client data
typedef struct client {
	// File descriptor to write into and to read from
	int fd;
	// An array of int for client answers
	int *answers;
	// Before user entered a name, he cannot issue commands
	// 0 - not in game
	// 1 - in game
	// 2 - completed game
	short state;
	// At most 128 char including the terminator '\0'
	char *usrname;
	// Designated buffer for user input
	char *buf;
	// Pointer to the current end-of-buf position
	char *after;
	// How many bytes in buf
	int inbuf;
	// How much room left in buf
	int bufleft;
	// Pointer to the next client node
	struct client *next;
} Client;

// Utility methods
void error(char *msg);
int validate_user(char *name);
char *alloc_str(int size);
Client *addClient(int fd);
void removeClient(Client *cl, Client *head);
int net_newline_location(char *buf, int inbuf);
int process_args(int, char **, QNode **, Node *, struct client *, struct client *);
int tokenize(char *, char **);
Node *existing_user(char* name, QNode *current);
// Server methods
void bindAndListen(int port);
int acceptConn();

#endif
