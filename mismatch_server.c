/* TCP server for the best-mismatch app */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <syslog.h>

#define SERVICE "best-mismatch" /* Name of TCP service */
#define BUFSIZE 1024
#define BACKLOG 5

int port = 12345; // TODO: change this to take an argument

// TODO: move to the client header
typedef struct Client {
  int fd;
  struct in_addr ipaddr;
  struct client *next;
} Client;

static char greeting[] = "Welcome to The Best Mismatch!\r\n";

static void grimReadper(int sig);
static void addClient(int fd, struct in_addr addr);
static void removeClient(int fd);
static void broadcast(char *msg, int size);

int main(int argc, char **argv)
{

  return 0;
}

static void grimReadper(int sig){
  int savedErrno; /* Save 'errno' in case changed here */

  savedErrno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    continue;
  errno = savedErrno
}