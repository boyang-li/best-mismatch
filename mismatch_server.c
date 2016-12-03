/* TCP server for the best-mismatch app */

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
#define MAX_USRNAME 128

// Use a linked list to maintain client data
typedef struct client {
    // File descriptor to write into and to read from
    int fd;
    // An array of int for client answers
    int *answers;
    // Before user entered a name, he cannot issue commands
    short state;
    // At most 128 char including the terminator '\0'
	char usrname[MAX_USRNAME];
    char buf[BUFSIZE];
    // Pointer to the current end-of-buf position
    int inbuf;
    // Pointer to the next client node
    struct client *next;
} Client;

void error(char *msg);
void bindAndListen(int port);
void acceptConn();
void addClient(int fd);
void removeClient(Client *cl);

int lfd; // This is the listening fd
short port = 12345; // TODO: change this to take an argument
Client *cur_cl = NULL; /* The current client node */
int clnum = 0;  /* server counts the number of connected clients */

char *greeting = "Welcome.\r\n";
char user_prompt[] = "What is your user name?\r\n";

int main(int argc, char **argv) {
    Client *cl = NULL; /* Current connected client */

    /* Configure server socket, bind and listen, abort on errors */
    bindAndListen(port);

    /* Main server loop, can only be stopped by signal kill */
    while (1) {
        int maxfd = lfd; /* First set maxfd to be listenfd */
        fd_set fdlist; /* A list for all client fds */
        FD_ZERO(&fdlist); /* Clear all entries from the set */
        FD_SET(lfd, &fdlist); /* Add listenfd to teh set */

        /* Filling up the set with client fds */
        for (cl = cur_cl; cl; cl = cl->next) {
            FD_SET(cl->fd, &fdlist);
            /* Update the maxfd to be the highest-numbered fd */
            if (cl->fd > maxfd) {
                maxfd = cl->fd;
            }
        }

        if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
            error("select");
        } else {
            printf("Selected fd = %d\n", maxfd);
            for (cl = cur_cl; cl; cl = cl->next) {
                printf("looking for the client with correct fd...\n");
                if (cl && FD_ISSET(cl->fd, &fdlist))
                    break;
            }
            /*
             * it's not very likely that more than one client will drop at
             * once, we process only one each select() for now;
             */
            if (cl) { // Returning client
                printf("Removing existing client %s...\n", cl->usrname);
                removeClient(cl);
            }

            if (FD_ISSET(lfd, &fdlist)) { // New client
                printf("Connecting new client...\n");

                /* The listen fd has data, accept client connection */
                acceptConn();



            } else {
                fprintf(stderr, "Shouldn't have happened!\n");
                exit(1);
            }

        }

    }

    return 0;
}

/* Wrapper for perror */
void error(char *msg) {
  perror(msg);
  exit(1);
}

/* Configure server socket, bind and listen, abort on errors */
void bindAndListen(int port) {
    if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        error("socket");
    }

    /* Allow re-using the sticky server ports */
    int optval = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (void *) &optval,
        sizeof(int)) == -1) {
        error("setsockopt - REUSEADDR");
    }

    struct sockaddr_in saddr; /* server's addr */

    /* Setup the server's Internet address */
    memset(&saddr, '\0', sizeof saddr);

    /* Configure Internet address */
    saddr.sin_family = AF_INET;

    /* Configure server's IP address */
    saddr.sin_addr.s_addr = INADDR_ANY;

    /* This is the port to listen */
    saddr.sin_port = htons(port);

    /* bind: associate this socket with a port */
    if (bind(lfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        error("bind");
    }

    /* listen: make this socket ready to accept connection requests */
    if (listen(lfd, BACKLOG)) {
        error("listen");
    }

    fprintf(stdout, "listenfd %d Listening on %d...\n", lfd, port);
}

void acceptConn() {
    int fd;
    struct sockaddr_in caddr; /* Client addr */
    socklen_t socklen = sizeof(caddr);
    int len;
    char buf[BUFSIZE];
    // char answer[BUFSIZE];

    if ((fd = accept(lfd, (struct sockaddr*) &caddr, &socklen)) < 0) {
        error("accept");
    } else {
        printf("Connection from %s!\n", inet_ntoa(caddr.sin_addr));

        printf("Client fd = %d\n", fd);
        // addClient(fd);
        write(fd, user_prompt, sizeof user_prompt - 1);


        /* This is the same, except there's nothing to unlink. */
        // close(*fd);
    }
}

void addClient(int fd) {
    // char *usrname = malloc(MAX_USRNAME);
    // char *buf = malloc(BUFSIZE);
    // int *answers = malloc(3*sizeof(int));

    // Client *c = malloc(sizeof(Client));
    // if (!c) {
    //     fprintf(stderr, "Out of memory!\n");
    //     exit(1);
    // }
    // fflush(stdout);
    // c->fd = fd;
    // c->answers = answers;
    // c->state = 0;
    // c->buf = buf;
    // c->inbuf = 0;
    // c->next = cur_cl;
    // cur_cl = c;
    // clnum++;
}

void removeClient(Client *cl) {
    // Client **cl; // Array of clients
    // Traverse the the array untill we find our client
    // for (cl = &cur_cl; *cl && (*cl)->fd != fd; cl = &(*cl)->next);
    if (cl) {
        Client *next_cl = cl->next;
        fprintf(stdout, "Removing client fd %d...\n", cl->fd);
        fflush(stdout);
        free(cl);
        cl = next_cl;
        clnum--;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but it's not found\n", cl->fd);
        fflush(stderr);
    }
}

int net_newline_location(char *buf, int inbuf) {
	int i;
	for (i = 0; i < inbuf - 1; i++){
		if ((buf[i] == '\r') && (buf[i + 1] == '\n')){
			return i;
		}
	}
	return -1;
}
