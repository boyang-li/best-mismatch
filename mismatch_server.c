/* TCP server for the best-mismatch app */
#include "app.h"

int lfd; /* The listening fd */
int clnum = 0;  /* server counts the number of connected clients */
char greeting[] = "Welcome.\r\nGo ahead and enter user commands>\r\n";
char user_prompt[] = "What is your user name?\r\n";
char name_error[] = "ERROR: User name must be at least 8 and at most "
            "128 characters, and can only contain alphanumeric "
            "characters.\r\n";
QNode *root = NULL;
Node *interests = NULL;
Client *cl_head = NULL; /* The head client node */
Client *cl_tail = NULL; /* The tail client node */

int main(int argc, char **argv) {
	if (argc < 2) {
        printf ("To run the program ./mismatch_server <name of input file>\n");
        return 1;
    }

    // read interests from file
    interests = get_list_from_file (argv[1]);

    if (interests == NULL)
        return 1;

    // build question tree
    root = add_next_level (root,  interests);

    //TODO: remove after testing
    print_qtree (root, 0);

	/* Configure server socket, bind and listen, abort on errors */
	short port = 55692; // TODO: change this to take an argument
	bindAndListen(port);

	char buf[BUFSIZE]; /* Buffer string to store a whole line of command */
	int nbytes; /* How many bytes we add to buffer */
	int inbuf; /* how many bytes currently in buffer? */
	int bufleft = sizeof(buf); /* how much room left in buffer? */
	char *after; /* pointer to position after the (valid) data in buf */
	int where; /* location of network newline */

	/* Main server loop, can only be stopped by signal kill */
	while (1) {
		int cfd; /* This is the client fd */
		int maxfd = lfd; /* First set maxfd to be listenfd */
		fd_set fdlist; /* A list for all client fds */
		FD_ZERO(&fdlist); /* Clear all entries from the set */
		FD_SET(lfd, &fdlist); /* Add listenfd to teh set */
		Client *cl = NULL; /* This is the current connected client */

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
			for (cl = cl_head; cl; cl = cl->next) {
				if (cl && FD_ISSET(cl->fd, &fdlist))
					break;
			}

			printf("lfd: %d\n", lfd);
			printf("maxfd: %d\n", maxfd);
			if (cl)
				printf("active cl fd: %d\n", cl->fd);

			/* Client connection
			 * it's not very likely that more than one client will drop at
			 * once, we process only one each select() for now;
			 */
			if (FD_ISSET(lfd, &fdlist)) {
				/* The listen fd has data, accept client connection */
				cfd = acceptConn();

				// First, ask the client for his username
				write(cfd, user_prompt, sizeof user_prompt - 1);

				/* Partial-read
				 * Read the buffer until end-of-line, and get the whole line
				 * for processing.
				 */
				inbuf = 0; // buffer is empty; has no bytes
				bufleft = sizeof(buf); // begin with amount of the whole buffer
      			after = buf; // start writing at beginning of buf

				while ((nbytes = read(cfd, after, bufleft)) > 0) {
					//amount of bytes read into buffer
					inbuf += nbytes;

					// call find_network_newline, store result in 'where'
					where = net_newline_location(buf,inbuf);

					if (where >= 0) { // OK. we have a full line
						//replace \r with \0
						buf[where] = '\0';
						buf[where+1] = '\0';

						//TODO: remove it after testing
						printf("Line extracted: %s\n", buf);
						int cmd_argc;
						char *cmd_argv[INPUT_ARG_MAX_NUM];

						/* Line-process
						 * Verify the client before processing any command.
						 */
						if (cl) { // Active client
							/* Check client state
							 * 1 - took test; 0 - didn't take test
							 */
							printf("Active client: %s!\n", cl->usrname);

							//TODO: Tokenize command and process them
							cmd_argc = tokenize(buf, cmd_argv);

							// process_args(cmd_argc, cmd_argv, &root, interests,
		 				// 				cl, cl_head);

						} else if (validate_user(buf) == 1) {
							// Add new client to list
							cl = addClient(cfd, buf);
							if (!cl_head) {
						    	printf("The new client is head.\n");
						    	cl_head = cl;
						    	cl_tail = cl;
						    } else {
						    	printf("Append the new client to list.\n");
						    	cl->next = cl_tail->next;
						    	cl_tail->next = cl;
						    }

							if (cl) {
								write(cfd, greeting, sizeof greeting - 1);
							    clnum++;
							} else { // This shouldn't happen
								fprintf(stderr, "Failed to add client!\n");
						        exit(1);
							}
						} else {
							write(cfd, name_error, sizeof name_error - 1);
							write(cfd, user_prompt, sizeof user_prompt - 1);
						} // Line-process ends

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
				// close(fd);

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

int acceptConn() {
	int fd;
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
    // First, search for client with the same username
	Client *c = NULL;
	for (c = cl_head; c; c = c->next) {
		printf("looking for the client with name: %s\n", buf);
		if (strcmp(c->usrname, buf) == 0) {
			printf("found client: %s\n", c->usrname);
			return c;
		}
	}

	printf("creating new client with name: %s\n", buf);
    c = malloc(sizeof(Client));
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
