#include <ctype.h>
#include "app.h"

// int *play_game(char* name, QNode *root, Node *interests);
int validate_answer(char *answer);
void wrap_up();
void print_mismatches(Node *list, char *name);
char *question_prompt = "Do you like %s? (y/n)\n";
char test_msg[] = "Collecting your interests\r\n";

//QNode *root = NULL;
//Node *interests = NULL;
Node *user_list = NULL;
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

int process_answer(Client *cl, int answer, QNode *root, Node *interests) {
 	
	
 	QNode *prev, *curr;
    prev = curr = root;
 	Node *i = interests;
	int j;
	int elements = sizeof(cl->answers)/sizeof(int);
	int count=0;
	while(j<elements){
		if(answers[j] == 1 || answers[j] == 0){
			count++;
		}
		j++;
	}
	cl->answers[count] = answer;
	
	while(i<elements){
		if
		prev = curr;
		curr = find_branch(curr, cl->answers[i]);
		i++;
		}

 	if (rc == 1) { // Yes
 		/* code */
 	} else { // No

 	}
 }

// int *play_game(Client *cl, char *answer, QNode *root, Node *interests){
// 	// char answer[MAX_LINE];
// 	// int *ans_arr;


// 	QNode *prev, *curr;
//     prev = curr = root;
// 	Node *i = interests;
// 	int ans;
// 	int index=0;
// 	ans_arr = (int*) malloc(sizeof(int));
// 	while (i){
// 		printf(question_prompt, i->str);

// 		// read answer to prompt
//         scanf("%s", answer);
//         ans = validate_answer(answer);

//         // if answer if not valid, continue to loop
//         if (ans == 2)
// 			continue;
// 		if (ans == -2){
// 			break;
// 		}

// 		prev = curr;
//         curr = find_branch(curr, ans);
// 		ans_arr[index] = ans;
// 		ans_arr = realloc(ans_arr, ((index+1)*sizeof(int)));
//         i = i->next;
// 		index += 1;
//     }
// 	// add user to the end of the list

// 	int *answers[(sizeof(ans_arr) - 1)];
// 	if (ans!=-2){
// 		*answers = ans_arr;

// 		user_list = prev->children[ans].fchild;
// 		prev->children[ans].fchild = add_user(user_list, name);
// 		printf("Test complete.");
// 	}else{
// 		*answers[0] = -2;
// 		printf("Test prematurely exitted. Answers not saved");
// 	}
// 	free(ans_arr);
// 	return *answers;
// }

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
		 struct client *cl, struct client *head) {
	QNode *qtree = *root;
	// 0 - continue; -1 - quit; 1 - no username


	if (cmd_argc <= 0) {
		return 0;

	} else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
		/* Return an appropriate value to denote that the specified
		 * user is now need to be disconnected. */
		return -1; //done in main loop fdset is not passed into here

	} else if (strcmp(cl->usrname, "") == 0 && cmd_argc == 1) {
		return 1;
	} else if (strcmp(cmd_argv[0], "do_test") == 0 && cmd_argc == 1) {
		/* The specified user is ready to start answering questions. You
		 * need to make sure that the user answers each question only
		 * once.
		 */
		if(cl->state == 2){
			write(cl->fd,"You've already taken the test!\r\n", 33);
		} else if(cl->state == 1){
			write(cl->fd,"You are already writing the test!\r\n", 35);
		} else {
			write(cl->fd, test_msg, sizeof test_msg - 1);
			//print first question...

			cl->state = 1;
		}

		return 0;

	} else if (cmd_argc == 1 && validate_answer(cmd_argv[0]) != 2)) {
		// check if user is in game
		if(cl->state == 0 || cl->state == 2){
			//print invalid
			return 0;
		}else{
			int status = process_answer(cl, validate_answer(cmd_argv[0]), qtree, interests);
			if (status == 2){
				cl->state = 2;
			} 
		}
		return 0;
	}


    } else if (strcmp(cmd_argv[0], "get_all") == 0 && cmd_argc == 1 &&
		cl->state == 2) {
		/* Send the list of best mismatches related to the specified
		 * user. If the user has not taked the test yet, return the
		 * corresponding error value (different than 0 and -1).
		 */

		if(cl->state == 0){
			return -2;
		}
		int *ans_list[sizeof(cl->answers)];
		*ans_list = cl->answers;
		int i=0;
		QNode *prev, *curr;
		prev = qtree;
		curr = qtree;
		int ans;
		int elements = sizeof(ans_list)/sizeof(int);
		while(i<elements){
			if(*ans_list[i] == 1){
				ans = 0;
			}else{
				ans = 1;
			}
			prev = curr;
			curr = find_branch(curr, ans);
			i++;
		}
		user_list = prev->children[ans].fchild;
		print_mismatches(user_list, cl->usrname);
		return 0;

	} else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc == 3) {
		/* Send the specified message stored in cmd_argv[2] to the user
		 * stored in cmd_argv[1].
		 */
		int sizebuf = sizeof(cmd_argv[2]);
		//create new string to hold message
		//new string needs to be larger by one to account for '\r\n'
		char msg[sizebuf+1];
		strcpy(msg,cmd_argv[2]);
		msg[sizebuf]='\r';
		msg[sizeof(msg)]='\n';
		//TODO remove later
		printf("%s",msg);
		//search client linked list for client
		while(head!=NULL){
			if(strcmp(head->usrname, cmd_argv[1])==0){
				write(head->fd, cmd_argv[2], sizebuf);
				return 0;
			}
			head = head->next;
		}
		return -1;
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
/*
void wrap_up(){
    //end of main loop - the user typed "q"
    free_list (interests);
    free_qtree(qtree);

    exit(0);
}
*/
int validate_answer(char *answer){
	if (strcmp(answer, "quit") == 0){
		return -2;
	}

    if (strlen(answer) > 3){
    	//TODO
        printf("%s", "invalid answer");
        return 2;
    }

    if (answer[0] == 'n' || answer[0] == 'N')
        return 0;

    if (answer[0] == 'y' || answer[0] == 'Y')
        return 1;

    return 2;
}

// print list of potential mismatches for user
void print_mismatches(Node *list, char *name){
    int mismatch = 0;


    // iterate over user list and count the number of mismatchs
    while (list) {
	// ignore this user
        if (strcmp(list->str, name)) {
            mismatch++;

	    // if this is the first mismatch found, print successful message
            if (mismatch == 1)
                printf("Here are you best mismatches:\n" );

	    // if mismatch was found, print his/her name
            printf("%s\n", list->str);
        }

        list = list->next;
    }

    if (mismatch == 0){
        printf("No completing personalities found. Please try again later\n");
    }
}

Node *existing_user(char* name, QNode *current){
        //very slightly modified find_user from A2 solution code
        //returns Null if user is not found, the node if found.
    if (current->node_type == REGULAR){

        Node *head;
        // look for the node (recursively) in the 0 subtree
        head = find_user(current->children[0].qchild, name);
        if (head)
            return head;

        // look for the node (recursively) in the 1 subtree
        head = find_user(current->children[1].qchild, name);
        if (head)
            return head;

    // current is a leaf node
    } else {
        // look for the user in the 0 child
        Node *head = current->children[0].fchild;

        while (head != NULL) {
            if (strcmp(head->str, name) == 0)
                return current->children[0].fchild;
            head = head->next;
        }

        // look for the user in the 1 child
        head = current->children[1].fchild;

        while (head != NULL) {
            if (strcmp(head->str, name) == 0)
                return current->children[1].fchild;
            head = head->next;
        }
    }

    return NULL;
}

int active_user(char *name, struct client *cl, struct client *head){
	client *cur;
	for(cur=head, cur->next, cur=cur->next){
		if (strcmp(cur->usrname, name) == 0){
			return 0;
		}
	}
	return 1;
}
