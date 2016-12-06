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
