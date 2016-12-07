/* TCP server for the best-mismatch app */
#include "app.h"

int lfd; // The listening fd
int clnum = 0;  /* server counts the number of connected clients */
short port = 12345; // TODO: change this to take an argument
char greeting[] = "Welcome.\r\nGo ahead and enter user commands>\r\n";
char user_prompt[] = "What is your user name?\r\n";
char name_error[] = "ERROR: User name must be at least 8 and at most "
            "128 characters, and can only contain alphanumeric "
            "characters.\r\n";

int main(int argc, char **argv) {
	/* Configure server socket, bind and listen, abort on errors */
	bindAndListen(port);

	int fd; /*  */
	char buf[BUFSIZE]; /* Buffer string to store a whole line of command */
	int nbytes; /* How many bytes we add to buffer */
	int inbuf; /* how many bytes currently in buffer? */
	int bufleft = sizeof(buf); /* how much room left in buffer? */
	char *after; /* pointer to position after the (valid) data in buf */
	int where; /* location of network newline */
	Client *cl_head = NULL; /* The head client node */

	/* Main server loop, can only be stopped by signal kill */
	while (1) {
		int maxfd = lfd; /* First set maxfd to be listenfd */
		fd_set fdlist; /* A list for all client fds */
		FD_ZERO(&fdlist); /* Clear all entries from the set */
		FD_SET(lfd, &fdlist); /* Add listenfd to teh set */

		/* This is the current connected client */
		Client *cl = NULL;

		/* Filling up the set with client fds */
		for (cl = cl_head; cl; cl = cl->next) {
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
			for (cl = cl_head; cl; cl = cl->next) {
				printf("looking for the client with correct fd...\n");
				if (cl && FD_ISSET(cl->fd, &fdlist))
					break;
			}

			/* Client connection
			 * it's not very likely that more than one client will drop at
			 * once, we process only one each select() for now;
			 */
			if (FD_ISSET(lfd, &fdlist)) {
				/* The listen fd has data, accept client connection */
				fd = acceptConn(fd);

				// First, ask the client for his username
				write(fd, user_prompt, sizeof user_prompt - 1);

				/* Partial-read
				 * Read the buffer until end-of-line, and get the whole line
				 * for processing.
				 */
				inbuf = 0; // buffer is empty; has no bytes
				bufleft = sizeof(buf); // begin with amount of the whole buffer
      			after = buf; // start writing at beginning of buf

				while ((nbytes = read(fd, after, bufleft)) > 0) {
					//amount of bytes read into buffer
					inbuf += nbytes;

					// call find_network_newline, store result in 'where'
					where = net_newline_location(buf,inbuf);

					if (where >= 0) { // OK. we have a full line
						//replace \r with \0
						buf[where] = '\0';
						buf[where+1] = '\0';

						//TODO: remove it after testing
						printf("Next message: %s\n", buf);

						/* Line-process
						 * Verify the client before processing any command.
						 */
						if (cl) { // Client verified
							/* Check client state
							 * 0 - not in test; 1 - in test
							 */
							printf("Hi, %s!\n", cl->usrname);

							if (cl->state == 0) { // Not in test
								//TODO: Tokenize command and process them
							} else { // In test
								//TODO: Process the answers
							}
						} else { // Client not verified
							// Look for client by username
							for (cl = cl_head; cl; cl = cl->next) {
								if (strcmp(cl->usrname, buf) == 0) {
									break;
								}
							}

							if (cl){ // Found the client
								write(fd, greeting, sizeof greeting - 1);
							} else if (validate_user(buf) == 1) {
								// Add new client to list
								cl = addClient(fd, buf);
								if (cl) {
								    clnum++;
								} else { // This shouldn't happen
									fprintf(stderr, "Failed to add client!\n");
							        exit(1);
								}
							} else {
								write(fd, name_error, sizeof name_error - 1);
								write(fd, user_prompt, sizeof user_prompt - 1);
							}
						}

						// Process message Ends---------------------------

						// Skip the '\0\0' terminators, 'where' is the number
						// of bytes in the full line
						where += 2;

						// Remove the full line from the buffer, in memory
						memmove(buf, buf + where, inbuf);

						// Remove bytes from inbuf as removing the full line
						inbuf -= where;

						// Buffer gains room as removing the full line
						bufleft += where;
					}

					// Buffer loses room after read
					bufleft -= nbytes;

					if (bufleft > 0){
						// update after, in preparation for the next read
		        		after = &buf[inbuf];
					} else {
						// Buffer is full, there is no next read.
						fprintf(stderr, "Buffer overflow!\n");

						//TODO: deal with buffer overflow properly, client is
						// prompted with error message, and program continues
						break;
					}
				}
				// close fd so that the other side knows
				close(fd);

				/* End of partial-read */
			} else {
				fprintf(stderr, "Shouldn't have happened!\n");
				exit(1);
			}

		}

	}

	return 0;
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

int acceptConn(int fd) {
	struct sockaddr_in caddr; /* Client addr */
	socklen_t socklen = sizeof(caddr);

	if ((fd = accept(lfd, (struct sockaddr*) &caddr, &socklen)) < 0) {
		error("accept");
	} else {
		printf("Connection from %s!\n", inet_ntoa(caddr.sin_addr));
	}

	return fd;
}

Client *addClient(int fd, char *buf) {
    Client *c = malloc(sizeof(Client));
    if (!c) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }

    int size = strlen(buf) + 1;
    c->usrname = alloc_str(size);
    strncpy(c->usrname, buf, size);
    c->fd = fd;
    c->buf = buf;
    c->inbuf = 0;
    c->state = 0;
    c->answers = NULL;
    c->next = NULL;

    return c;
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
