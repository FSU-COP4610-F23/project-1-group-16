/*
Questions for TA:
how do we move forward in a directory with cd and ls
cd dir
cd ./dir
cd ~/dir
make file 
*/


#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

int main()
{
	while (1) {
		printf("%s@%s:%s>", getenv("USER"), getenv("MACHINE"), getenv("PWD"));

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input); //will be deleted later

		tokenlist *tokens = get_tokens(input);
		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]); //this line is for testing and should be deleted later
		}

		//this is where we take the tokens and do what we need to do with them 
		doCode(tokens);

		free(input);
		free_tokens(tokens);
	}

	return 0;
}

char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens) {
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}

void doCode(tokenlist *tokens){
	if(tokens->size == 0){ //if no input, move on and ask for input again 
		return;
	}
	//get first token
	char *first = tokens->items[0]; //grab first
	tilde(first);
	//echo
	if(!(strcmp(first, "echo"))){
		if(tokens->size == 1){ //nothing after echo so just return empty line
			printf("\n");
		}
		else {
			int i = 1;
			while(tokens->items[i] != NULL){
				char *next = tokens->items[i]; //grab next
				if(next[0] == '$'){ //if echoing an environment variable
					char* sec = next;
					sec++; //remove $
					if(getenv(sec) != NULL){
						printf("%s ", getenv(sec)); //prints the echo
					}
				}
				else{
					printf("%s ", next); //echo without $ just echos the user 
				}
				i++; //go to next word to echo 
			}
			printf("\n"); //end of echoing
		}
	} //end of echo
	//ls
	else if((strcmp(first, "ls")) == 0){
		char *cmd[] = {"ls", NULL};
		int pid = fork();
		if (pid == 0) {
			if (execv("/bin/ls", cmd) == -1) {
				perror("execv");
				exit(1);
			}
		} else if (pid > 0) {
			waitpid(pid, NULL, 0);
		} else {
			perror("fork");
			exit(1);
		}
	}
	else if((strcmp(first, "cd")) == 0){
		if((tokens->size == 1) || (strcmp(tokens->items[1], "~") == 0)){ //nothing after cd, or ~ after, so make PWD, HOME
			setenv("PWD", getenv("HOME"), 1); //1 means if PWD exists, it is updated
			chdir(getenv("HOME"));
			printf("done\n");
		}
		else if(strcmp(tokens->items[1], "..") == 0){ //cd ..(go back one directory)
			char current_directory[4096]; //buffer to store the current directory
			if (getcwd(current_directory, sizeof(current_directory)) != NULL) { //grab current working directory and put in current_directory
				char* last_slash = strrchr(current_directory, '/'); //returns a pointer to the last occurrence of "/" in current_directory
				//might want to change above function to strtok() because that is what he uses in the shell PP
				if (last_slash != NULL) { //need to have found a "/"
					*last_slash = '\0'; //null-terminate the string at the last slash
					setenv("PWD", current_directory, 1); //change print working directory
					chdir(current_directory);
				} else {
					printf("Already at the root directory; cannot go up.\n");
				}
			} 
			else {
				perror("Error");
			}
		}
		//need to add if cd into folder into folder 
		else if (strcmp(tokens->items[1], "./bin") == 0){ //if cd ~
			//cd ~(something after)
			//check if file directory exists before going into it 
			// . is current directory
			// ~ is home directory
			printf("got in cd .\n");
			DIR* dir = opendir("bin");
			printf("got here\n");
			if (dir) {
				/* Directory exists. */
				setenv("PWD", tokens->items[1], 1); //change print working directory
				chdir(tokens->items[1]);
				closedir(dir);
			} else if (ENOENT == errno) {
				/* Directory does not exist. */
			} else {
				/* opendir() failed for some other reason. */
			}
		}
	}

}

void tilde(char *token){
	if(token[0] == '~'){
			char* replace = token; //get the token that has ~
			replace++; //get rid of ~
			char* adding = "$HOME";
			char expanded[strlen(adding) + strlen(token)];
			sprintf(expanded, "%s%s", adding, token+1);
			strcpy(token, expanded);
			printf("tok is %s\n", token);
		}
}
