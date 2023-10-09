/*
Questions for TA:
are we allowed to use setenv - YES
jobs
piping
background processing
timeout executable

parent can know if child successfully executed execv with WEXITSTATUS



Need: 
exit printing last 3 valid
background processing (Amelia)
double pipe (becky)
jobs (Olivia)
Readme
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

		// Background Processing
		if(strcmp(tokens->items[tokens->size-1], "&") == 0){
			int status;
			int job_number = 1; // Initialize job number
			// tokens->items[tokens->size-1] = NULL; // NULL out the &
			
			pid_t pid = fork();

			if(pid == 0) {
				execv(tokens->items[0], tokens->items);
				printf("Inside child process\n");
			}
			else if(pid > 0){
				waitpid(pid, &status, WNOHANG);
				printf("Inside parent process\n");
				printf("[%d] [%d]\n", job_number, getpid());
			}
			else{
				perror("fork");
				exit(1);
			}
		}

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
			//cause fork where child handles piping and parent waits for child
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
				if(handleExternal(tokens, inRedirection, 
						outRedirection, 
						out_file, in_file, 
						foundPipe) == true){
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


	tokenlist *cmd1 = new_tokenlist();
	for(int i = 0; i < loc1; i++){
		cmd1->items = 
			(char **)realloc(cmd1->items, (i + 2) * sizeof(char*));
		cmd1->items[i] = (char *)malloc(strlen(tokens->items[i]) + 1);
		cmd1->items[i + 1] = NULL;
		strcpy(cmd1->items[i], tokens->items[i]);
		cmd1->size ++;
	}
	
	//put the first command into newTokens
	tokenlist *cmd2_3 = new_tokenlist();
	int place1 = 0;
	for(int i = loc1+1; i < tokens->size; i++){
		cmd2_3->items = (char **)realloc(cmd2_3->items, 
				(place1 + 2) * sizeof(char*));
		cmd2_3->items[place1] = 
			(char *)malloc(strlen(tokens->items[i]) + 1);
		cmd2_3->items[place1 + 1] = NULL;
		// printf("loc is %d, i is %d\n", loc1, i);
		strcpy(cmd2_3->items[place1], tokens->items[i]);
		cmd2_3->size ++;
		place1++;
	}

	//put the first command into cmd2
	tokenlist *cmd2 = new_tokenlist();
	for(int i = 0; i < loc1; i++){
		cmd2->items = (char **)realloc(cmd2->items, 
				(i + 2) * sizeof(char*));
		cmd2->items[i] = (char *)malloc(strlen(cmd2_3->items[i]) + 1);
		cmd2->items[i + 1] = NULL;
		strcpy(cmd2->items[i], cmd2_3->items[i]);
		cmd2->size ++;
	}

	//get only the tokens needed into newTokens
	//get command 3 into cmd3
	tokenlist *cmd3 = new_tokenlist();
	int place2 = 0;
	for(int i = loc1+1; i < cmd2_3->size; i++){
		cmd3->items = (char **)realloc(cmd3->items, 
				(place2 + 2) * sizeof(char*));
		cmd3->items[place2] = 
			(char *)malloc(strlen(cmd2_3->items[i]) + 1);
		cmd3->items[place2 + 1] = NULL;
		printf("loc is %d, i is %d\n", loc1, i);
		strcpy(cmd3->items[place2], cmd2_3->items[i]);
		cmd3->size ++;
		place2++;
	}


	printf("\n");
	for (int i = 0; i < cmd2_3->size; i++) {
		//this line is for testing and should be deleted later
		printf("cmd2_3 token %d: (%s)\n", i, cmd2_3->items[i]); 
	}
	printf("\n");
	for (int i = 0; i < cmd1->size; i++) {
		//this line is for testing and should be deleted later
		printf("cmd1 token %d: (%s)\n", i, cmd1->items[i]); 
	}
	printf("\n");
	for (int i = 0; i < cmd2->size; i++) {
		//this line is for testing and should be deleted later
		printf("cmd2 token %d: (%s)\n", i, cmd2->items[i]); 
	}
	printf("\n");
	for (int i = 0; i < cmd3->size; i++) {
		//this line is for testing and should be deleted later
		printf("cmd3 token %d: (%s)\n", i, cmd3->items[i]); 
	}


	
	int saveOut = dup(STDOUT_FILENO);
	int saveIn = dup(STDIN_FILENO);

	pid_t pid = fork();
	if(pid == 0){
		int fd[2];
		pipe(fd);
		dup2(fd[1], STDOUT_FILENO); //out to fd[1]
		close(fd[0]);
		close(fd[1]);

		cmd1->items[0] = pathSearch(cmd1->items[0]);
		if (execv(cmd1->items[0], cmd1->items) == -1) { 
			perror("execv");
			exit(1);
		}


		pid_t pid1 = fork();
		if(pid1 == 0){
			//child 1
			int fd2[2];
			pipe(fd2);

			
			pid_t pid2 = fork();
			if(pid2 == 0){
				//child 2
				printf("child 2\n");
				dup2(fd[0], STDIN_FILENO); //in from original fd[0]
				dup2(fd2[1], STDOUT_FILENO); //out to new out fd[1]
				close(fd2[0]);
				close(fd2[1]);

				cmd2->items[0] = pathSearch(cmd2->items[0]);
				if (execv(cmd2->items[0], cmd2->items) == -1) { 
					perror("execv");
					exit(1);
				}
			}
			else{
				//parent 2
				dup2(fd2[0], STDIN_FILENO); //in from fd2[0]
			 	dup2(saveOut, STDOUT_FILENO); //out to original out
				close(fd2[0]);
				close(fd2[1]);
				printf("Line 282\n");


				cmd3->items[0] = pathSearch(cmd3->items[0]);
				if (execv(cmd3->items[0], cmd3->items) == -1) { 
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
	else{
		waitpid(pid, NULL, 0);
	}
}

void singlePipe(tokenlist *tokens, int loc1, int loc2){
	//found pipe
	//cause fork where child handles piping and parent waits for child
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
				dup2(fd[1], STDOUT_FILENO); //print out is now to fd[1]
				close(fd[0]);
				close(fd[1]);
				
				//put the first command into newTokens
				tokenlist *newTokens = new_tokenlist();
				for(int i = 0; i < loc1; i++){
					newTokens->items = 
							(char **)realloc(newTokens->items, 
							(i + 2) * sizeof(char*));
					newTokens->items[i] = 
						(char *)malloc(strlen(tokens->items[i]) + 1);
					newTokens->items[i + 1] = NULL;
					strcpy(newTokens->items[i], tokens->items[i]);
					newTokens->size ++;
				}

				newTokens->items[0] = pathSearch(newTokens->items[0]);
				if (execv(newTokens->items[0], newTokens->items) == -1){
					perror("execv");
					exit(1);
				}
			}
			else{
				//parent 2
				dup2(fd[0], STDIN_FILENO); //in is now from fd[0]
				close(fd[0]);
				close(fd[1]);

				//get only the tokens needed into newTokens
				//get command 2 into newTokens
				tokenlist *newTokens = new_tokenlist();
				int place = 0;
				for(int i = loc1+1; i < tokens->size; i++){
					newTokens->items = 
						(char **)realloc(newTokens->items, 
						(place + 2) * sizeof(char*));
					newTokens->items[place] = 
						(char *)malloc(strlen(tokens->items[i]) + 1);
					newTokens->items[place + 1] = NULL;
					printf("loc is %d, i is %d\n", loc1, i);
					strcpy(newTokens->items[place], tokens->items[i]);
					newTokens->size ++;
					place++;
				}

				newTokens->items[0] = pathSearch(newTokens->items[0]);
				if (execv(newTokens->items[0], newTokens->items) == -1){
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

void addCommandToValid(commandHistory *history, char *command){
	if(history->count < 3){
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
	printf("Last (3) valid commands:\n");
	for(int i = 0; i < history->count; i++){
		printf("[%d]: %s\n", i + 1, history->commands[i]);
	}

	if(history->count == 0)
	{
		printf("There are no valid commands.\n");
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
		setenv("PWD", getcwd(s, 200), 1);
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

	if(open(out_file, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0600) == -1){
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
int handleExternal(tokenlist *tokens, bool inRedirection, bool outRedirection, 
					char *out_file, char *in_file, bool foundPipe){
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

		//if execv is successful, it terminates child
		if (execv(tokens->items[0], tokens->items) == -1) { 
			perror("execv");
			exit(1);
		}
	} else if (pid > 0) { //if parent thread
		waitpid(pid, &status, 0); 
		if (WIFEXITED(status)) {
            return !WEXITSTATUS(status);
        } else {
            return false; // Child process did not exit normally
        }
		//do something with status here 
		//and return 0 or 1 if child successfully executed execv
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

// void background(tokenlist *tokens){
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
// 				//indicates that the parent process shouldn't wait
//             pid_t result = waitpid(backgroundsJobs[i].pid, &stat, WNOHANG);     
//             if(result == backgroundsJobs[i].pid)
//             {
//                 backgroundsJobs[i].status = 1;
//                 printf("[%d] done %s\n", backgroundsJobs[i].numJobs, 
// 					backgroundsJobs[i].cmdLine);
//             }
            
//         }
//     }
// }