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
#define MAX_USRNAME 128

// Use a linked list to maintain client data
typedef struct client {
    // File descriptor to write into and to read from
    int fd;
    // Client's IP address
    struct in_addr ipaddr;
    // An array of int for client answers
    int *answers;
    // Before user entered a name, he cannot issue commands
    int state;
    // At most 128 char including the terminator '\0'
	char usrname[MAX_USRNAME];
    // Pointer to the current end-of-buf position
    int inbuf;
    // Pointer to the next client node
    struct client *next;
} Client;

void error(char *msg);
void bindAndListen(int port);
void acceptConn();
void addClient(int fd, struct in_addr addr);
void removeClient(int fd);
void broadcast(char *msg, int size);

int lfd; // This is the listening fd
short port = 12345; // TODO: change this to take an argument
char greeting[] = "Welcome to The Best Mismatch!\r\n";
Client *cur_cl = NULL; /* The current client node */
int clnum = 0;  /* server counts the number of connected clients */

int main(int argc, char **argv) {
    /* Configure server socket, bind and listen, abort on errors */
    bindAndListen(port);

    /* Main server loop, can only be stopped by signal kill */
    while (1) {
        int maxfd = lfd; /* The listening fd */
        fd_set fdlist; /* A set for all fds to be selected */
        FD_ZERO(&fdlist); /* Clear all entries from the set */
        FD_SET(lfd, &fdlist); /* Add the listening fd to the set */
        Client *cl;

        printf("Reset fd set\n");

        for (cl = top; cl; cl = cl->next) {
            FD_SET(cl->fd, &fdlist);
            if (cl->fd > maxfd) {
                maxfd = cl->fd;
            }
        }

        printf("maxfd = %d\n", maxfd);
        int rv;
        rv = select(maxfd + 1, &fdlist, NULL, NULL, NULL); /* No timeout */
        if (rv < 0) {
            error("select");
        } else {
            for (cl = top; cl; cl = cl->next) {
                if (FD_ISSET(cl->fd, &fdlist))
                    break;
                /*
                 * it's not very likely that more than one client will drop at
                 * once, we process only one each select() for now;
                 */
                // if (cl)
                    /* might remove cl from set, so can't be in the loop */
                    // TODO: client input validation goes here...
                    // checkInput(cl);

                /* The listen fd has data, accept client connection */
                if (FD_ISSET(lfd, &fdlist))
                    acceptConn();

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

    fprintf(stdout, "fd %d Listening on %d...\n", lfd, port);
}

void acceptConn() {
    printf("Accepting connection...\n");

    int fd;
    struct sockaddr_in caddr; /* Client addr */
    socklen_t socklen = sizeof(caddr);
    int len;
    char buf[BUFSIZE];
    char answer[ANSSIZE];

    if ((fd = accept(lfd, (struct sockaddr*) &caddr, &socklen)) < 0) {
        error("accept");
    } else {
        printf("connection from %s\n", inet_ntoa(caddr.sin_addr));
        addClient(fd, caddr.sin_addr);

        // WIP: try echo to client for now

        /* And this is the same too. */
        if ((len = read(fd, buf, sizeof(buf) - 1)) < 0) {
            error("read");
        }
        buf[len] = '\0';
        /*
         * Here we should be converting from the network newline convention to the
         * unix newline convention, if the string can contain newlines.
         */
        fprintf(stdout, "The other side said: %s\n", buf);

        sprintf(answer, "You said %s", buf);

        //write it back

        if ((len = write(fd, answer, strlen(answer))) != strlen(answer) ) {
            error("write");
        }

        /* This is the same, except there's nothing to unlink. */
        close(fd);
    }
}

void addClient(int fd, struct in_addr addr) {
    Client *cl = malloc(sizeof(Client));
    if (!cl) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    fprintf(stdout, "Adding client %s\n", inet_ntoa(addr));
    fflush(stdout);
    cl->fd = fd;
    cl->ipaddr = addr;
    cl->next = cur_cl;
    cur_cl = cl;
    clnum++;
}

void removeClient(int fd) {
    Client **cl; // Array of clients
    // Traverse the the array untill we find our client
    for (cl = &cur_cl; *cl && (*cl)->fd != fd; cl = &(*cl)->next);
    if (*cl) {
        Client *next_cl = (*cl)->next;
        fprintf(stdout, "Removing client %s\n", inet_ntoa((*cl)->ipaddr));
        fflush(stdout);
        free(*cl);
        *cl = next_cl;
        clnum--;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but it's not found\n", fd);
        fflush(stderr);
    }
}

void broadcast(char *msg, int size) {
    Client *cl;

    /* should probably check write() return value and perhaps remove client */
    for (cl = top; cl; cl = cl->next) {
        write(cl->fd, msg, size);
    }
}