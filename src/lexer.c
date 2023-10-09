/*
Questions for TA:
are we allowed to use setenv - YES
jobs
piping
background processing
timeout executable

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
#include <fcntl.h>
#include <sys/stat.h>

int main()
{
	commandHistory history;
	history.count = 0;
	bool inRedirection; 
	bool outRedirection; 
	char *out_file;
	char *in_file;
	bool foundPipe;

	while (1) {
		bool executed = false;
		inRedirection = false; 
		outRedirection = false; 
		foundPipe = false;
		
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

		tilde(tokens); //tilde expansion
		
		// If no entry
		if(tokens->size == 0){ //if no input, move on and ask for input again 
			printf("\n"); //print a line
			executed = true;
		}



		// if(strcmp(tokens.items[tokens.size-1], "&") == 0){
		// 	pid_t pid = fork();
		// 	if(pid == 0){
		// 		//child
		// 		//print thing
		// 		//execute cmd
		// 		//print 
		// 	}
		// 	else if(pid > 0){
		// 		//parent
		// 		free(input);
		// 		free_tokens(tokens);
		// 		continue; //this should continue to top of while loop
		// 	}
		// 	else{
		// 		perror("fork");
		// 		exit(1);
		// 	}
		// }

		int pipeLoc1 = -1;
		int pipeLoc2 = -1;
		for(int i = 0; i < tokens->size; i++){
            if((strcmp(tokens->items[i], "|") == 0)){
				if(pipeLoc1 == -1){
					pipeLoc1 = i;
				}
				else{
					pipeLoc2 = i;
				}
			}
		}

		// This is where we handle piping in main
		// for(int i = 0; i < tokens->size; i++){
		if(pipeLoc1 != -1){
			//found pipe
			//cause fork where child handles piping and parent waits for child to finish
			singlePipe(tokens, pipeLoc1, pipeLoc2); 
			free(input);
			free_tokens(tokens);
			foundPipe = true;
        }

		if(!foundPipe){
			for(int i = 1; i < tokens->size; i++){
				if((strcmp(tokens->items[i], ">") == 0)){
					// Output redirection
					outRedirection = true;
					out_file = tokens->items[i+1];
					tokens->items[i] = NULL;
					continue;
				}
				if((strcmp(tokens->items[i], "<") == 0)){
					// Input redirection
					inRedirection = true;
					in_file = tokens->items[i+1];
					tokens->items[i] = NULL;
					continue;
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
				if(handleExternal(tokens, inRedirection, outRedirection, out_file, in_file, foundPipe) == true){
					addCommandToValid(&history, input);
				}
			}
			
			free(input);
			free_tokens(tokens);
		}
	}

	return 0;
} // END OF MAIN

void doublePipe(tokenlist *tokens, int loc1, int loc2){
	int fd[2];
	pipe(fd);
	pid_t pid = fork();
	if(pid == 0){

		tokenlist *newTokens2 = new_tokenlist();
		for(int i = 0; i < loc1; i++){
			newTokens2->items = (char **)realloc(newTokens2->items, (i + 2) * sizeof(char*));
			newTokens2->items[i] = (char *)malloc(strlen(tokens->items[i]) + 1);
			newTokens2->items[i + 1] = NULL;
			strcpy(newTokens2->items[i], tokens->items[i]);
			newTokens2->size ++;
		}
		
		//put the first command into newTokens
		tokenlist *newTokens = new_tokenlist();
		int place = 0;
		for(int i = loc1+1; i < tokens->size; i++){
			newTokens->items = (char **)realloc(newTokens->items, (place + 2) * sizeof(char*));
			newTokens->items[place] = (char *)malloc(strlen(tokens->items[i]) + 1);
			newTokens->items[place + 1] = NULL;
			// printf("loc is %d, i is %d\n", loc1, i);
			strcpy(newTokens->items[place], tokens->items[i]);
			newTokens->size ++;
			place++;
		}

		for (int i = 0; i < newTokens2->size; i++) {
			//this line is for testing and should be deleted later
			printf("Line 184 token %d: (%s)\n", i, newTokens2->items[i]); 
		}
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);

		newTokens2->items[0] = pathSearch(newTokens2->items[0]);
		if (execv(newTokens2->items[0], newTokens2->items) == -1) { 
			perror("execv");
			exit(1);
		}


		printf("Line 197\n");
		singlePipe(newTokens, loc2 - loc1, -1);
		printf("Line 199\n");


	}
	else{
		
		wait(pid);
	}
}

void singlePipe(tokenlist *tokens, int loc1, int loc2){
	//found pipe
	//cause fork where child handles piping and parent waits for child to finish
	if(loc2 != -1){
		doublePipe(tokens, loc1, loc2);
	}
	else{
		pid_t pid1 = fork();
		if(pid1 == 0){
			//child 1

			int fd[2];
			pipe(fd);
			
			pid_t pid2 = fork();
			if(pid2 == 0){
				//child 2
				printf("child 2\n");
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				
				//put the first command into newTokens
				tokenlist *newTokens = new_tokenlist();
				for(int i = 0; i < loc1; i++){
					newTokens->items = (char **)realloc(newTokens->items, (i + 2) * sizeof(char*));
					newTokens->items[i] = (char *)malloc(strlen(tokens->items[i]) + 1);
					newTokens->items[i + 1] = NULL;
					strcpy(newTokens->items[i], tokens->items[i]);
					newTokens->size ++;
				}

				newTokens->items[0] = pathSearch(newTokens->items[0]);
				if (execv(newTokens->items[0], newTokens->items) == -1) { 
					perror("execv");
					exit(1);
				}
			}
			else{
				//parent 2
				dup2(fd[0], STDIN_FILENO);
				close(fd[0]);
				close(fd[1]);

				//get only the tokens needed into newTokens
				//get command 2 into newTokens
				tokenlist *newTokens = new_tokenlist();
				int place = 0;
				for(int i = loc1+1; i < tokens->size; i++){
					newTokens->items = (char **)realloc(newTokens->items, (place + 2) * sizeof(char*));
					newTokens->items[place] = (char *)malloc(strlen(tokens->items[i]) + 1);
					newTokens->items[place + 1] = NULL;
					printf("loc is %d, i is %d\n", loc1, i);
					strcpy(newTokens->items[place], tokens->items[i]);
					newTokens->size ++;
					place++;
				}

				newTokens->items[0] = pathSearch(newTokens->items[0]);
				if (execv(newTokens->items[0], newTokens->items) == -1) { 
					perror("execv");
					exit(1);
				}
			}
		}
		else if (pid1 > 0){
			//parent 1 
			//wait for child
			waitpid(pid1, NULL, 0);
		}
		else{
			perror("fork");
			exit(1);
		}
	}
}


bool doPipe(tokenlist *tokens, int loc){
	printf("inside doPipe\n");
	
	// create array for pipe file descriptors
	int fd[2];
	
	// create read and write ends of pipe
	pipe(fd); // Could've done this - int pipe(int fd[2]);
	printf("fd[0] = %d\nfd[1] = %d\n", fd[0], fd[1]); // fd[0] = 3, fd[1] = 4
	
	// create new process
	pid_t pid = fork();

	// child process
	if(pid == 0) {
		printf("inside child\n");

		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		// execv here somewhere?
		exit(0);
		
		// // initialize command
		// char *x[2];
		// x[0] = tokens->items[0];
		// x[1] = NULL;

		// // replace stdout with write end of pipe
		// dup2(fd[1], STDOUT_FILENO);
		
		// // close read end of pipe
		// close(fd[0]);
		
		// // close duplicate reference to write end of pipe
		// close(fd[1]);

		for(int i = loc; i > 0; i--){
			free(tokens->items);
			tokens->size--;
		}
		if (execv(tokens->items[0], tokens->items) == -1) { 
			perror("execv");
			exit(1);
		}
		// execvp(x[0], x); // Can't use
	}
	// parent process
	else {
		printf("inside parent\n");

		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		close(fd[1]);
		exit(0);

		// // initialize command
		// char *x[3];
		// x[0] = "wc";
		// x[1] = "-l";
		// x[2] = NULL;

		// // replace stdin with read end of pipe
		// dup2(fd[0], STDIN_FILENO);
		
		// // close read end of pipe
		// close(fd[0]);
		
		// // close duplicate reference to write end of pipe
		// close(fd[1]);

		// // if (execv(tokens->items[loc], tokens->items+loc) == -1) { 
		// // 	perror("execv");
		// // 	exit(1);
		// // }

		// // remember to use execv() instead of execvp() for the project!
		// execvp(x[0], x);
	}
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
		return 3;
	}
	else if(strcmp(tokens->items[0], "jobs") == 0){
		//jobs();
		return 1;
	}
	return 0;
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

	/* 
	sort < alphabet.txt > output.txt - THIS WORKS
	The result from this command is that alphabet.txt remains unchanged but output.txt now has the sorted contents of alphabet.txt

	sort > alphabet.txt < output.txt - THIS WORKS
	The result from this command is that output.txt remains unchanged but alphabet.txt now has the contents of output.txt
	*/

}
/*
return 0 is not valid
return 1 is valid
*/
int handleExternal(tokenlist *tokens, bool inRedirection, bool outRedirection, char *out_file, char *in_file, bool foundPipe){
	tokens->items[0] = pathSearch(tokens->items[0]);
	//pass above into execv
	int status;
	pid_t pid = fork(); //splits into two threads that are both at this line
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
		// if(foundPipe){
		// 	if(!doPipe(tokens, tokens->items[loc])){

		// 	}
		// }
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

// void background(){
// 	int status;
// 	pid_t pid = fork();

// 	if(pid == 0) {
// 		sleep(3);
// 		printf("Child process\n");
// 	}
// 	else {
// 		waitpid(pid, &status, WNOHANG);
// 		printf("Parent process\n");
// 	}
// }

// typedef struct {
//     // int numJobs = 0;
//     pid_t pid;
//     // char cmdLine[100];
// 	tokenlist tokens;
//     int status = 0;     //0 is not running, 1 is running
// } Job;

// Job backgroundsJobs[10];

// void addJob(pid_t pid, const char* cmdLine)
// {
//     for(int i=0; i<sizeof(backgroundsJobs); i++)
//     {
//         if(backgroundsJobs[i].status == 0) //found open job space
//         {
//             Job j = backgroundsJobs[i];
//             // j.pid = pid;
//             strcpy(j.cmdLine, cmdLine);
//             j.status = 1;
//             // backgroundsJobs[j.numJobs - 1]= j;
//         }
//         else
//         {
//             printf("Maximum number of background jobs reached.\n");
//         }
//     }
// }

// void checkJobs()
// {
//     for(int i = 0; i<10; i++)
//     {
//         if(backgroundsJobs[i].status == 0)
//         {
//             int stat;
//             pid_t result = waitpid(backgroundsJobs[i].pid, &stat, WNOHANG);     //indicates that the parent process shouldn't wait
//             if(result == backgroundsJobs[i].pid)
//             {
//                 backgroundsJobs[i].status = 1;
//                 printf("[%d] done %s\n", backgroundsJobs[i].numJobs, backgroundsJobs[i].cmdLine);
//             }
            
//         }
//     }
// }