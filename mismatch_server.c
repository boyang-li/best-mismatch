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

#define BUFSIZE 1024
#define BACKLOG 5
#define ANSSIZE 128

short port = 12345; // TODO: change this to take an argument
char greeting[] = "Welcome to The Best Mismatch!\r\n";

// TODO: move to the client header
typedef struct Client {
    int fd;
    struct in_addr ipaddr;
    struct client *next;
} Client;

Client *top = NULL;
int clnum = 0;  /* server counts the number of connected clients */

void error(char *msg);
void bindAndListen(int *fd, int port);
void addClient(int *fd, struct in_addr addr);
void removeClient(int fd);
void broadcast(char *msg, int size);

int main(int argc, char **argv)
{
    int sfd, cfd;
    char answer[ANSSIZE];

    /* Configure server socket, bind and listen, abort on errors */
    bindAndListen(&sfd, port);

    /* Main server loop, can only be stopped by signal kill */
    while (1) {
        int lfd = sfd; /* The listening fd */
        fd_set readfds; /* A set for all fds to be selected */
        FD_ZERO(&readfds); /* Clear all entries from the set */
        FD_SET(sfd, &readfds); /* Add the listening fd to the set */

        for (cl = clhead; cl; cl = cl->next) {
            FD_SET(cl->fd, &readfds);
            if (cl->fd > lfd) {
                lfd = cl->fd;
            }
        }

        int rv;
        rv = select(lfd + 1, &readfds, NULL, NULL, NULL); /* No timeout */
        if (rv < 0) {
            error("select");
        } else {
            for (cl = clhead; cl; cl = cl->next) {
                if (FD_ISSET(cl->fd, &readfds)) {
                    break;
                }
                /*
                 * it's not very likely that more than one client will drop at
                 * once, we process only one each select() for now;
                 */
                if (cl) {
                    /* might remove cl from set, so can't be in the loop */
                    // TODO: client input validation goes here...
                    checkInput(cl);
                }
                /* The listen fd has data, accept client connection */
                if (FD_ISSET(lfd, &readfds)) {
                    acceptConn(&sfd, &cfd);
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
void bindAndListen(int *fd, int port) {
    if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        error("socket");
    }

    /* Allow re-using the sticky server ports */
    int optval = 1;
    if (setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, (void *) &optval,
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
    if (bind(*fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        error("bind");
    }

    /* listen: make this socket ready to accept connection requests */
    if (listen(*fd, BACKLOG)) {
        error("listen");
    }

    fprintf(stdout, "Listening on %d...\n", port);
}

void acceptConn(int *sfd, int *cfd) {
    struct sockaddr_in caddr; /* Client addr */
    socklen_t socklen = sizeof(caddr);
    int len, i, c;
    char buf[BUFSIZE];

    if ((*cfd = accept(*sfd, (struct sockaddr *)&caddr, &socklen)) < 0) {
        error("accept");
    } else {
        printf("connection from %s\n", inet_ntoa(caddr.sin_addr));

        addClient(cfd, caddr.sin_addr);

        // WIP: try echo to client for now
    }
}

void addClient(int *fd, struct in_addr addr) {
    Client *cl = malloc(sizeof(Client));
    if (!cl) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    fprintf(stdout, "Adding client %s\n", inet_ntoa(addr));
    fflush(stdout);
    cl->fd = *fd;
    cl->ipaddr = addr;
    cl->next = top;
    top = cl;
    clnum++;
}