/*
Questions for TA:
how do we move forward in a directory with cd and ls
make file 
can we treat echo as internal
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

		//do tilde here
		// tilde(tokens);
		for(int i = 0; i < tokens->size; i++){
			tilde(tokens->items[i]);
		}
		//this is where we take the tokens and do what we need to do with them 
		// doCode(tokens);
		//if no entry
		if(tokens->size == 0){ //if no input, move on and ask for input again 
			return; //print a line
		}
		//if internal command
		else if(isInternal(tokens->items[0])){
			//internal command(tokens)
			handleInternal(tokens);
		}
		else{
			//external command(tokens)
			handleExternal(tokens);
		}

		free(input);
		free_tokens(tokens);
	}

	return 0;
}

bool isInternal(char* token){
	if((strcmp(token, "echo") == 0) || 
		(strcmp(token, "cd") == 0) || 
		(strcmp(token, "exit") == 0) ||
		(strcmp(token, "jobs") == 0)){
			return true;
		}
	return false;
}

void handleInternal(tokenlist *tokens){
	if(!(strcmp(tokens->items[0], "echo"))){
		echo(tokens);
	} //end of echo
	else if(!(strcmp(tokens->items[0], "cd"))){
		cd(tokens);
	}
}

void cd(tokenlist *tokens){
	// char *second = tokens->items[1]; // this is what is after cd 
	// printf("this is second %s\n", second);
	// if(chdir(second) != 0)
	// {
	// 	perror("chdir() to token after cd failed");
	// }
	chdir("..");
}

void echo(tokenlist *tokens){
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
}

void handleExternal(tokenlist *tokens){
	tokens->items[0] = pathSearch(tokens->items[0]);
	//pass above into execv
	int pid = fork();
	if (pid == 0) {
		printf("passing in %s\n", tokens->items[0]);
		if (execv(tokens->items[0], tokens->items) == -1) {
			perror("execv");
			exit(1);
		}
		else{
			printf("input invalid\n");
		}
	} else if (pid > 0) {
		waitpid(pid, NULL, 0);
	} else {
		perror("fork");
		exit(1);
	}
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

char * pathSearch (char * token){ //pass in one token and change it
	tilde(token);
	char * path = getenv("PATH");
	char * copy = malloc(strlen(path));
	strcpy(copy, path);
	printf("copy is %s\n", copy);
	strtok(copy, ":"); //deliminate path with :
	char * cmd = NULL;
	printf("copy is %s\n", copy);
	while(copy != NULL){
		// printf("inside while\n");
		cmd = realloc(cmd, copy + strlen(token) + 2); //2 because of the "/" and end null pointer (/0)
		strcpy(cmd, copy);
		strcat (cmd, "/");
		strcat(cmd, token);
		// printf("cmd is %s\n", cmd);
		if(access(cmd, F_OK) ==  0){
			// printf("cmd is found and is %s\n", cmd);
			free(token);
			return cmd;
		}
		copy = strtok(NULL, ":");
	}
}

char* tilde(char *token){
	// for(int i = 0; i < tokens->size; i++){
		// printf("i is %s\n", i);
		// char * token = tokens->items[i];
		// if(token[0] == '~'){
		// 	char* homePath = getenv("HOME");
		// 	char *new_path = (char *)malloc(strlen(homePath) + strlen(token));
		// 	token = (char *)malloc(strlen(homePath) + strlen(token));
		// 	strcpy(new_path, homePath);
		// 	strcat(new_path, token + 1);
		// 	strcpy(token, new_path);
		// 	printf("inside tilde. New path is %s\n", token);
		// }
		char * path = malloc(strlen(getenv("HOME")) + strlen(token) +1);
		strcpy(path, getenv("HOME"));
		strcat(path, token+1);
		return path;
		
	// }

}


void doCode(tokenlist *tokens){

	//get first token
	char *first = tokens->items[0]; //grab first
	tilde(first);
	//ls
	if((strcmp(first, "ls")) == 0){
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
	// cd
	else if((strcmp(first, "cd")) == 0){
		// if((tokens->size == 1) || (strcmp(tokens->items[1], "~") == 0)){ //just cd, or ~ after, so make PWD, HOME
		// 	// setenv("PWD", getenv("HOME"), 1); //1 means if PWD exists, it is updated
		// 	chdir(getenv("HOME"));
		// 	printf("done\n");
		// }

		char *second = tokens->items[1]; // this is what is after cd 
		printf("this is second %s", second);
		if(chdir(second) != 0)
		{
			perror("chdir() to token after cd failed");
		}
		
		// else if(strcmp(tokens->items[1], "..") == 0){ //cd ..(go back one directory)
		// 	char current_directory[4096]; //buffer to store the current directory
		// 	if (getcwd(current_directory, sizeof(current_directory)) != NULL) { //grab current working directory and put in current_directory
		// 		char* last_slash = strrchr(current_directory, '/'); //returns a pointer to the last occurrence of "/" in current_directory
		// 		//might want to change above function to strtok() because that is what he uses in the shell PP
		// 		if (last_slash != NULL) { //need to have found a "/"
		// 			*last_slash = '\0'; //null-terminate the string at the last slash
		// 			setenv("PWD", current_directory, 1); //change print working directory
		// 			chdir(current_directory);
		// 		} else {
		// 			printf("Already at the root directory; cannot go up.\n");
		// 		}
		// 	} 
		// 	else {
		// 		perror("Error");
		// 	}
		// }
		// //need to add if cd into folder into folder 
		// else if (strcmp(tokens->items[1], "./bin") == 0){ //if cd ~
		// 	//cd ~(something after)
		// 	//check if file directory exists before going into it 
		// 	// . is current directory
		// 	// ~ is home directory
		// 	printf("got in cd .\n");
		// 	// DIR* dir = opendir("bin"); // DIR is a directory stream
		// 	printf("got here\n");
		// 	//if (dir) {
		// 		/* Directory exists. */
		// 		//setenv("PWD", tokens->items[1], 1); //change print working directory
		// 		chdir(tokens->items[1]);
		// 		//closedir(dir);
		// 	// } else if (ENOENT == errno) {
		// 	// 	/* Directory does not exist. */
		// 	// } else {
		// 	// 	/* opendir() failed for some other reason. */
		// 	// }

		// }
	}

}

