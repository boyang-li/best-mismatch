// #include <ctype.h>
#include "app.h"

/*
 * Utility methods
 */

/* Wrapper for perror */
void error(char *msg) {
	perror(msg);
	exit(1);
}

/* Check if an username is valid */
int validate_user(char *name){
	int valid = 1;

	int len = strlen(name);
	if (len < 8 || len > 128)
		valid = 0;

	for (int i = 0; i < len; i++){
		if (!isalnum(name[i])){
			valid = 0;
			break;
		}
	}

	return valid;
}

/* Allocate memory dynamically for string, remember to free it after! */
char *alloc_str(int size) {
	char *s = malloc(sizeof(char) * size);
	// Zero out the string
  memset(s, '\0', sizeof(char) * size);
  return s;
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

/*
 * Read and process commands
 */
int process_args(int cmd_argc, char **cmd_argv, QNode **root, Node *interests,
		 struct client *current_client, struct client *head) {
	QNode *qtree = *root;

	if (cmd_argc <= 0) {
		return 0;

	} else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
		/* Return an appropriate value to denote that the specified
		 * user is now need to be disconnected. */
		return -1;

	} else if (strcmp(cmd_argv[0], "do_test") == 0 && cmd_argc == 1) {
		/* The specified user is ready to start answering questions. You
		 * need to make sure that the user answers each question only
		 * once.
		 */

	} else if (strcmp(cmd_argv[0], "get_all") == 0 && cmd_argc == 1) {
		/* Send the list of best mismatches related to the specified
		 * user. If the user has not taked the test yet, return the
		 * corresponding error value (different than 0 and -1).
		 */

	} else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc == 3) {
		/* Send the specified message stored in cmd_argv[2] to the user
		 * stored in cmd_argv[1].
		 */
	}
	else {
		/* The input message is not properly formatted. */
		error("Incorrect syntax");
	}
	return 0;
}

/*
 * Tokenize the command stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);

    while (next_token != NULL) {
        cmd_argv[cmd_argc] = next_token;
        ++cmd_argc;

	if(cmd_argc < (INPUT_ARG_MAX_NUM - 1))
	    next_token = strtok(NULL, DELIM);
	else
	    break;
    }

    if (cmd_argc == (INPUT_ARG_MAX_NUM - 1)) {
	cmd_argv[cmd_argc] = strtok(NULL, "");
	if(cmd_argv[cmd_argc] != NULL)
	    ++cmd_argc;
    }

    return cmd_argc;
}
