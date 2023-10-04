/*
Questions for TA:
make file 
are we allowed to use setenv - YES
jobs
piping
background processing
timeout executable
external timeout executable - need to create new program that has nothing to do with this program

mytimeout
pass in command arguments (4 arguments)
fork()
execv(argv[2], argv +2)
//parent
wait how many seconds specified then kill
parent kill child
kill(pid, SIG_kill)


parent can know if child successfully executed execv with WEXITSTATUS
*/

#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> // For I/O Redirection
#include <sys/stat.h> // For I/O Redirection

int main()
{
	commandHistory history;
	history.count = 0;
	bool inRedirection; 
	bool outRedirection; 
	char *out_file;
	char *in_file;

	while (1) {
		bool executed = false;
		inRedirection = false; 
		outRedirection = false; 
		
		printf("%s@%s:%s>", getenv("USER"), getenv("MACHINE"), getenv("PWD"));

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input); //will be deleted later

		tokenlist *tokens = get_tokens(input);
		
		for (int i = 0; i < tokens->size; i++) {
			//this line is for testing and should be deleted later
			printf("token %d: (%s)\n", i, tokens->items[i]); 
		}

		tilde(tokens);
		
		// If no entry
		if(tokens->size == 0){ //if no input, move on and ask for input again 
			printf("\n"); //print a line
			executed = true;
		}

		for(int j = 1; j < tokens->size; j++){
			if((strcmp(tokens->items[j], ">") == 0)){
				// Output redirection
				outRedirection = true;
				out_file = tokens->items[j+1];
				tokens->items[j] = NULL;
				break; //change to continue
			}
			if(strcmp(tokens->items[j], "<") == 0)
			{
				// Input redirection
				inRedirection = true;
				in_file = tokens->items[j+1];
				tokens->items[j] = NULL;
				break;
			}
		}

		// If internal command
		if(!executed){
			int internal = isInternal(tokens);
			if(internal != 0){ // If found internal
				if(internal == 1){ // If successful
					addCommandToValid(&history, input);
				}
				executed = true;
			}
			
			if(internal == 3){ // If exit
				displayLastThree(&history);
				exit(0);
			}
		}

		// If external command
		if(!executed){
			// External command
			if(handleExternal(tokens, inRedirection, outRedirection, out_file, in_file) == true){
				addCommandToValid(&history, input);
			}
		}
		
		free(input);
		free_tokens(tokens);
	}

	return 0;
} // END OF MAIN

void addCommandToValid(commandHistory *history, char *command){
	if(history->count<3){ //if we don't have three commands yet, simple add it on
		strcpy(history->commands[history->count], command);
		history->count++;
	}
	else{
		for(int i = 0; i < 2; i++){
			strcpy(history->commands[i], history->commands[i+1]);
		}
		strcpy(history->commands[2], command);
	}
}

void displayLastThree(commandHistory *history){
	// int start = (history->count > 3) ? history->count - 3 : 0;
	for(int i = 0; i < history->count; i++){
		printf("Command %d: %s\n", i + 1, history->commands[i]);
	}
}

/*
return 0 if not internal command
return 1 if internal command successful
return 2 if internal command not successful
return 3 if exit
*/
int isInternal(tokenlist *tokens){
	if((strcmp(tokens->items[0], "echo") == 0)){
		echo(tokens);
		return 1;
	}
	else if(strcmp(tokens->items[0], "cd") == 0){
		return cd(tokens);
		// return 1;
	}
	else if(strcmp(tokens->items[0], "exit") == 0){
		exitCommand();
		return 3;
	}
	else if(strcmp(tokens->items[0], "jobs") == 0){
		//jobs();
		return 1;
	}
	return 0;
}

void exitCommand(){
	//wait for other processes to finish //MORE CODE NEEDED HERE
}

int cd(tokenlist *tokens){
	if (chdir(tokens->items[1]) == 0){
		char s[200]; //200 should be max length of PWD
		setenv("PWD", getcwd(s, 200), 1); //1 means if PWD exists, it is updated
		return 1;
	}
	else {
		printf("errno");
		return 2;
	}
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


bool doOutRedirection(tokenlist *tokens, char *out_file){
	// Output redirection	
	close(STDOUT_FILENO); // Closes the file descriptor

	if(open(out_file, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0600) == -1){ // Open output file
		return false;
	}

	return true;
}

bool doInRedirection(tokenlist *tokens, char *in_file){
	// Input redirection	
	close(STDIN_FILENO); // Closes the file descriptor

	if(open(in_file, O_RDONLY | O_SYNC) == -1){ // Open input file
		return false;
	}

	return true;

}
/*
return 0 is not valid
return 1 is valid
*/
int handleExternal(tokenlist *tokens, bool inRedirection, bool outRedirection, char *out_file, char *in_file){
	tokens->items[0] = pathSearch(tokens->items[0]);
	//pass above into execv
	int status;
	int pid = fork(); //splits into two threads that are both at this line
	if (pid == 0) { //if child thread: 
		//if execv is successful, the child thread is terminated here 
		if(outRedirection){
			if(!doOutRedirection(tokens, out_file)){
				//redirection did not work
				printf("File redirection failed"); //may not need this
				exit(1);
			}
		}
		if(inRedirection){
			if(!doInRedirection(tokens, in_file)){
				printf("Input file redirection failed");
				exit(1);
			}
		}
		//if execv is successful, it terminates child
		if (execv(tokens->items[0], tokens->items) == -1) { 
			perror("execv");
			exit(1);
		}
	} else if (pid > 0) { //if parent thread
		waitpid(pid, &status, 0); //This NULL needs to change because it gets the if child successfully executed execv 
		//do something with status here and return 0 or 1 if child successfully executed execv
		//if status tell us child ran successfully, return 1, else return 0
		//if child was good
		return true;

	} else { //forking failed
		perror("fork");
		exit(1);
	}
	return false; //should never hit this line
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

char *pathSearch (char * token){ //pass in one token and change it
	// tilde(token);
	char * path = getenv("PATH");
	char * copy = malloc(strlen(path));
	strcpy(copy, path);
	strtok(copy, ":"); //deliminate path with :
	char * cmd = NULL;
	while(copy != NULL){
		//+ 2 --> "/" + end null terminator (/0)
		cmd = malloc(strlen(copy) + strlen(token) + 2); 
		strcpy(cmd, copy);
		strcat (cmd, "/");
		strcat(cmd, token);
		if(access(cmd, F_OK) ==  0){
			free(token);
			return cmd;
		}
		copy = strtok(NULL, ":");
	}
	return copy;
}

void tilde(tokenlist *tokens){
	for(size_t i = 0; i < tokens->size; i++){
		char*token = tokens->items[i];
		if(token[0] == '~'){ //check for tilde
			char* home = getenv("HOME"); //same home path
			//allocate new memory
			char *new_path = malloc(strlen(home) + strlen(token) -1);
			strcpy(new_path, home);
			strcat(new_path, token + 1); //concat what is needed
			free(tokens->items[i]); //free memory
			tokens->items[i] = new_path; //change the token
		}
	}
}