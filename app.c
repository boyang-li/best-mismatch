#include <ctype.h>
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

void process_partial(int fd){
	int nbytes;
	char buf[BUFSIZE];
	int inbuf = 0; // how many bytes currently in buffer?
	int bufleft = sizeof(buf); // how much room left in buffer?
	char *after; // pointer to position after the (valid) data in buf
	int nlloc; // location of network newline	// buffer is empty; has no bytes
    bufleft = sizeof(buf); // bufleft == capacity of the whole buffer
    after = buf;        // start writing at beginning of buf
	if ((nbytes = read(fd, after, bufleft)) > 0) {
    inbuf = inbuf + nbytes; //amount of bytes read into buffer
    nlloc = net_newline_location(buf,inbuf);

		if (nlloc >= 0) { //there is a netnewline terminated command to process
		//replace both \r and \n with \0
		buf[nlloc] = '\0';
		buf[nlloc+1] = '\0';

		/*TODO:process the commands here*/

		//remove previous command by replacing it with everything after the \r\n
		//shift everything up
		nlloc+=2;
		inbuf = inbuf - nlloc;
		//we are preparing to remove the previous command allong with the 2 '\0'
		//so inbuf is cleared
		memmove(buf, buf+nlloc,inbuf); //everything behind \0\0 is pushed to the front

		}
	}
}
