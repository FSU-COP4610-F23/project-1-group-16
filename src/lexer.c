/*
Questions for TA:
make file 
are we allowed to use setenv
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

int main()
{
	commandHistory history;
	history.count = 0;
	bool running = true;
	while (running) {
		bool executed = false;
		printf("running is %d\n", running);
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
		//this is where we take the tokens and do what we need to do with them 
		// doCode(tokens);
		//if no entry
		if(tokens->size == 0){ //if no input, move on and ask for input again 
			printf("\n"); //print a line
			executed = true;
		}
		//if internal command
		if(!executed) {
			int intern = isInternal(tokens, running);
			if(intern != 0){ //found internal
				if(intern == 1){
					addCommandToValid(&history, input);
				}
				executed = true;
			}
			printf("running is %d\n", running);
			if(intern == 3){
				//exit
				printf("found exit\n");
				displayLastThree(&history);
			}
		}
		if(!executed){
			printf("inside is external\n");
			//external command(tokens)
			if(handleExternal(tokens)){ //this needs to return bool
				addCommandToValid(&history, input);
			}
		}

		free(input);
		free_tokens(tokens);
	}

	return 0;
}

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
	printf("inside display last three\n");
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
int isInternal(tokenlist *tokens, bool running){
	printf("running is %d inside isInternal\n", running);
	printf("inside here line 68\n");
	if((strcmp(tokens->items[0], "echo") == 0)){
		echo(tokens);
		return 1;
	}
	else if(strcmp(tokens->items[0], "cd") == 0){
		return cd(tokens);
		// return 1;
	}
	else if(strcmp(tokens->items[0], "exit") == 0){
		exitCommand(running);
		return 3;
	}
	else if(strcmp(tokens->items[0], "jobs") == 0){
		//jobs();
		return 1;
	}
	return 0;
}

void exitCommand(bool running){
	printf("running is %d inside exitCommand\n", running);
	sleep(2); //wait for other processes to finish //2 might be wrong number
	//print last 3 valid commands
	running = false;
	printf("running is %d inside exitCommand\n", running);


}

int cd(tokenlist *tokens){
	if (chdir(tokens->items[1]) == 0){
		char s[100];
		setenv("PWD", getcwd(s, 100), 1); //1 means if PWD exists, it is updated
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

/*
return 0 is not valid
return 1 is valid
*/
int handleExternal(tokenlist *tokens){
	tokens->items[0] = pathSearch(tokens->items[0]);
	//pass above into execv
	bool valid = true;
	int pid = fork(); //splits into two threads that are both at this line
	if (pid == 0) { //if child thread: 
		printf("passing in %s\n", tokens->items[0]);
		//if execv is successful, the child thread is terminated here 
		if (execv(tokens->items[0], tokens->items) == -1) { 
			valid = false;
			printf("line 188\n");
			perror("execv");
			exit(1);
		}
		else{
			valid = false;
			printf("input invalid\n");
		}
		valid = false;
		printf("line 197\n");
		exit(1);
	} else if (pid > 0) { //if parent thread
		waitpid(pid, NULL, 0);

		if(valid){
			printf("valid is true\n");
		}
		else{
			printf("valid is false\n");
		}
		printf("line 199\n");
		// waitpid(-1, NULL, 0);
	} else { //forking failed
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
	// tilde(token);
	char * path = getenv("PATH");
	char * copy = malloc(strlen(path));
	strcpy(copy, path);
	printf("copy is %s\n", copy);
	strtok(copy, ":"); //deliminate path with :
	char * cmd = NULL;
	printf("copy is %s\n", copy);
	while(copy != NULL){
		// printf("inside while\n");
		//+ 2 --> "/" + end null terminator (/0)
		cmd = malloc(strlen(copy) + strlen(token) + 2); 
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
	return copy;
}

void tilde(tokenlist *tokens){
	for(size_t i = 0; i < tokens->size; i++){
		char*token = tokens->items[i];
		if(token[0] == '~'){ //check for tilde
			printf("found ~\n");
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


// void doCode(tokenlist *tokens){

// 	//get first token
// 	char *first = tokens->items[0]; //grab first
// 	tilde(first);
// 	//ls
// 	if((strcmp(first, "ls")) == 0){
// 		char *cmd[] = {"ls", NULL};
// 		int pid = fork();
// 		if (pid == 0) {
// 			if (execv("/bin/ls", cmd) == -1) {
// 				perror("execv");
// 				exit(1);
// 			}
// 		} else if (pid > 0) {
// 			waitpid(pid, NULL, 0);
// 		} else {
// 			perror("fork");
// 			exit(1);
// 		}
// 	}
// 	// cd
// 	else if((strcmp(first, "cd")) == 0){
//		// //just cd, or ~ after, so make PWD, HOME
// 		// if((tokens->size == 1) || (strcmp(tokens->items[1], "~") == 0)){ 
//		//	//1 means if PWD exists, it is updated
// 		// 	// setenv("PWD", getenv("HOME"), 1); 
// 		// 	chdir(getenv("HOME"));
// 		// 	printf("done\n");
// 		// }

// 		char *second = tokens->items[1]; // this is what is after cd 
// 		printf("this is second %s", second);
// 		if(chdir(second) != 0)
// 		{
// 			perror("chdir() to token after cd failed");
// 		}
		
// 		// else if(strcmp(tokens->items[1], "..") == 0){ //cd ..
// 		// 	char current_directory[4096]; //buffer to store the current directory
//		// 	//grab current working directory and put in current_directory
// 		// 	if (getcwd(current_directory, sizeof(current_directory)) != NULL) { 
// 		// 		//returns a pointer to the last occurrence of "/" in current_directory
// 		// 		char* last_slash = strrchr(current_directory, '/'); 
// 		// 		//might want to change to strtok() --> what he uses in the shell PP
// 		// 		if (last_slash != NULL) { //need to have found a "/"
// 		// 			*last_slash = '\0'; //null-terminate the string at the last slash
// 		// 			setenv("PWD", current_directory, 1);
// 		// 			chdir(current_directory);
// 		// 		} else {
// 		// 			printf("Already at the root directory; cannot go up.\n");
// 		// 		}
// 		// 	} 
// 		// 	else {
// 		// 		perror("Error");
// 		// 	}
// 		// }
// 		// //need to add if cd into folder into folder 
// 		// else if (strcmp(tokens->items[1], "./bin") == 0){ //if cd ~
// 		// 	//cd ~(something after)
// 		// 	//check if file directory exists before going into it 
// 		// 	// . is current directory
// 		// 	// ~ is home directory
// 		// 	printf("got in cd .\n");
// 		// 	// DIR* dir = opendir("bin"); // DIR is a directory stream
// 		// 	printf("got here\n");
// 		// 	//if (dir) {
// 		// 		/* Directory exists. */
// 		// 		//setenv("PWD", tokens->items[1], 1);
// 		// 		chdir(tokens->items[1]);
// 		// 		//closedir(dir);
// 		// 	// } else if (ENOENT == errno) {
// 		// 	// 	/* Directory does not exist. */
// 		// 	// } else {
// 		// 	// 	/* opendir() failed for some other reason. */
// 		// 	// }

// 		// }
// 	}

// }

