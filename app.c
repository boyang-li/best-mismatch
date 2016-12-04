#include <ctype.h>
#include "app.h"

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

char *alloc_str(int size) {
	char *s = malloc(sizeof(char) * size);
	// Zero out the string
  memset(s, '\0', sizeof(char) * size);
  return s;
}
