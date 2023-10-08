/*
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


#include <dirent.h>
#include <errno.h>
#include <fcntl.h> // For I/O Redirection
#include <sys/stat.h> // For I/O Redirection

char *pathSearch (char * token);

int main(int argc, char * argv[]){
    if(argc<=2){
        printf("3+ command line arguments were not passed in, execution failed\n");
        return 1;
    }

    // // Create an array of pointers for execv
    // char *exec_args[argc - 1];
    // for (int i = 2; i < argc; i++) {
    //     exec_args[i - 2] = argv[i];
    // }
    // exec_args[argc - 2] = NULL;
    
    int time = atoi(argv[1]);
    for(int i = 0; i < argc; i++){
        printf("argv[%d] is %s\n", i, argv[i]);
    }
    pid_t pid = fork();
    if(pid == 0){
        //child
        // printf("argv[2] is %s\n", argv[2]);
        argv[2] = pathSearch(argv[2]);
        printf("argv[2] is %s\n", argv[2]);
        // printf("argv + 2 is %s\n", argv+2);
        if (execv(argv[2], argv + 2) == -1) { 
			perror("execv");
			exit(1);
		}
    }
    else if (pid > 0){
        //parent
        // int time = argv[1];
        sleep(time);
        //this allows graceful termination of child
        kill(pid, SIGTERM);
        // Wait for the child process to fully finish
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return 1; // Child process did not exit normally
        }
        
    }
    else { //forking failed
		perror("fork");
		exit(1);
	}
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
		cmd = realloc(cmd, strlen(copy) + strlen(token) + 2); 
		strcpy(cmd, copy);
		strcat (cmd, "/");
		strcat(cmd, token);
		if(access(cmd, F_OK) ==  0){
			// free(token);
			return cmd;
		}
		copy = strtok(NULL, ":");
	}
    // free(cmd);
	return copy;
}

//gcc -g -Wall -std=c99 -c mytimeout.c -o mytimeout.o
//gcc -o mytimeout mytimeout.o

/*
To compile on terminal: 
g++ -c ___.h (this is class)
g++ -c ___.cpp (this is class) 
g++ -c ____.cpp (this is driver)
g++ ___.o ___.o -o application.exe (this is the 2nd the 3rd thing complied above) 
./application.exe

*/