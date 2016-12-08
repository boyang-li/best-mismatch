/* TCP server for the best-mismatch app */
#include "app.h"

// int clnum = 1;  /* server counts the number of connected clients */
char greeting[] = "Welcome.\r\nGo ahead and enter user commands>\r\n";
char user_prompt[] = "What is your user name?\r\n";
char name_error[] = "ERROR: User name must be at least 8 and at most "
			"128 characters, and can only contain alphanumeric "
			"characters.\r\n";
char ans_error[] = "ERROR: Answer must be one of 'y', 'n'.\r\n";
char welcomeback[] = "Welcome back.\r\n";

QNode *root = NULL;
Node *interests = NULL;
int interests_size;

Client *cl_head = NULL; /* The head client node */
Client *cl_tail = NULL; /* The tail client node */

int lfd;

int main(int argc, char **argv) {
	// Check for input file path
	if (argc < 2) {
		printf ("To run the program ./mismatch_server <name of input file>\n");
		return 1;
	}

	// read interests from file
	interests = get_list_from_file (argv[1]);
	if (interests == NULL)
		return 1;
	interests_size = get_list_size (interests);

	// build question tree
	root = add_next_level (root,  interests);

	//TODO: remove after testing
	print_qtree (root, 0);

	Client *cl = NULL;

	/* Configure server socket, bind and listen, abort on errors */
	bindAndListen();

	// char buf[BUFSIZE]; /* Buffer string to store a whole line of command */
	int nbytes; /* How many bytes we add to buffer */
	// int inbuf; /* how many bytes currently in buffer? */
	// int bufleft; /* how much room left in buffer? */
	// char *after; /* pointer to position after the (valid) data in buf */
	int where; /* location of network newline */

	/* Main server loop, can only be stopped by signal kill */
	while (1) {
		fd_set fdlist;
		int maxfd = lfd;
		FD_ZERO(&fdlist);
		FD_SET(lfd, &fdlist);

		/* Filling up the working set with client fds */
		for (cl = cl_head; cl; cl = cl->next) {
			// printf("Adding fd: %d to fdlist...\n", cl->fd);
			FD_SET(cl->fd, &fdlist);
			/* Update the maxfd to be the highest-numbered fd */
			if (cl->fd > maxfd) {
				maxfd = cl->fd;
			}
		}

		// printf("looping. maxfd: %d\n", maxfd);
		if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
			error("select");
		} else {
			if (FD_ISSET(lfd, &fdlist)) { // is listener
				int newfd = acceptConn();
				// FD_SET(newfd, &fdlist);
				cl = addClient(newfd);

				if (cl_tail) {
					cl->next = cl_tail->next;
					cl_tail->next = cl;
				} else {
					cl_head = cl;
					cl_tail = cl;
				}
				printf("Accepting new fd: %d\n", newfd);

				// Ask the client for his username
				write(newfd, user_prompt, sizeof user_prompt - 1);
			} else { // handle data from client
				for (cl = cl_head; cl; cl = cl->next) {
					if (cl && FD_ISSET(cl->fd, &fdlist)) {
						/* Partial-read
						 * Read the buffer until end-of-line, and get the whole line
						 * for processing.
						 */
						if ((nbytes = read(cl->fd, cl->after, cl->bufleft)) > 0) {
							//amount of bytes read into buffer
							cl->inbuf += nbytes;

							// call find_network_newline, store result in 'where'
							where = net_newline_location(cl->buf, cl->inbuf);

							if (where >= 0) { // OK. we have a full line
								//replace \r with \0
								cl->buf[where] = '\0';
								cl->buf[where+1] = '\0';

								//TODO: remove it after testing
								printf("Line extracted: %s\n", cl->buf);

								// Process args-------------------------
								int cmd_argc, rc;
								char *cmd_argv[INPUT_ARG_MAX_NUM];
								cmd_argc = tokenize(cl->buf, cmd_argv);

								rc = process_args(cmd_argc, cmd_argv, &root,
									interests, cl, cl_head);

								if (rc == -1) { // quit
									//close connection for client
									close(cl->fd);
									removeClient(cl, cl_head, cl_tail);
								} else if (rc == 1) { // no client name
									if (validate_user(cl->buf) == 1) {
										strncpy(cl->usrname, cl->buf, strlen(cl->buf)+1);

										if (existing_user(cl->buf, root) == NULL) {
											write(cl->fd, greeting, sizeof greeting - 1);
										} else {
											write(cl->fd, welcomeback, sizeof welcomeback - 1);
											cl->state = 2;
										}
									} else {
										// Ask the client for his username
										write(cl->fd, name_error, sizeof name_error - 1);
										write(cl->fd, user_prompt, sizeof user_prompt - 1);
									}
								}
								//---------------------------------------

								// Skip the '\0\0' terminators, 'where' is the number
								// of bytes in the full line
								where += 2;

								// Remove the full line from the buffer, in memory
								memmove(cl->buf, cl->buf + where, cl->inbuf);

								// Remove bytes from inbuf as removing the full line
								cl->inbuf -= where;

								// Buffer gains room as removing the full line
								cl->bufleft += where;
							}

							// Buffer loses room after read
							cl->bufleft -= nbytes;

							if (cl->bufleft > 0){
								// update after, in preparation for the next read
				        		cl->after = &((cl->buf)[cl->inbuf]);
							} else {
								// Buffer is full, there is no next read.
								fprintf(stderr, "Buffer overflow!\n");

								//TODO: deal with buffer overflow properly, client is
								// prompted with error message, and program continues
								break;
							}
						}


					}
				}
			}
				// close fd so that the other side knows
				// close(cfd);

				/* End of partial-read */


			// else {
			// 	fprintf(stderr, "Shouldn't have happened!\n");
			// 	exit(1);
			// }

		}

	}

	return 0;
}

/* Configure server socket, bind and listen, abort on errors */
void bindAndListen() {
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
	saddr.sin_port = htons(PORT);

	/* bind: associate this socket with a port */
	if (bind(lfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
		error("bind");
	}

	/* listen: make this socket ready to accept connection requests */
	if (listen(lfd, BACKLOG)) {
		error("listen");
	}

	fprintf(stdout, "listenfd %d Listening on %d...\n", lfd, PORT);
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

Client *addClient(int fd) {
	// First, search for client with the same username
	Client *c = NULL;
	// for (c = cl_head; c; c = c->next) {
	// 	printf("looking for the client with name: %s\n", buf);
	// 	if (strcmp(c->usrname, buf) == 0) {
	// 		printf("found client: %s\n", c->usrname);
	// 		return c;
	// 	}
	// }

	// printf("creating new client with name: %s\n", buf);
	c = malloc(sizeof(Client));
	if (!c) {
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}


	c->fd = fd;
	c->buf = alloc_str(BUFSIZE);
	c->usrname = alloc_str(NAME_SIZE);
	c->after = c->buf;
	c->inbuf = 0;
	c->bufleft = BUFSIZE;
	c->state = 0;
	c->answers = malloc(sizeof(int) * interests_size);

	return c;
}

void removeClient(Client *cl, Client *head, Client *tail) {
	// Traverse the the array untill we find our client
	if (head == cl){
		fprintf(stdout, "Removing head %d...\n", cl->fd);
		fflush(stdout);
		head = NULL;
		tail = NULL;
	} else {
		Client *cur;
		for (cur = head; cur->next && (cur->next)!=cl; cur = cur->next) {
			if (cur->next == cl) {
				fprintf(stdout, "Removing client fd %d...\n", cl->fd);
				fflush(stdout);
				cur->next = cur->next->next;

				break;
			}
		}
	}

	free(cl->buf);
	free(cl->usrname);
	free(cl->answers);
	free(cl);
}
