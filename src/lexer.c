#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	// char *user = getenv("USER");
	// char *machine = getenv("MACHINE");
	// char *pwd = getenv("PWD"); //parent working directory

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
				char *next = tokens->items[i]; //grab second
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
				i++;
			}
			printf("\n"); //end of echoing
		}
	} //end of echo
	//ls
	else if(!(strcmp(first, "ls"))){
		//get directory by env and then get entities 
	}

}

void tilde(char *token){
	if(token[0] == '~'){
			char* replace = token; //get the token that has ~
			printf("line 97\n");
			replace++; //get rid of ~
			printf("line 99\n");
			char* adding = "$HOME";
			printf("line 101\n");
			char expanded[strlen(adding) + strlen(token)];
			printf("line 104\n");
			sprintf(expanded, "%s%s", adding, token+1);
			printf("line 106\n");
			strcpy(token, expanded);
			printf("line 108\ntok is %s\n", token);
		}
}

/*
printf("token help\n");
		//if ~ then make it not 
		if(tok[0] == '~'){
			char* replace = tok; //get the token that has ~
			printf("line 97\n");
			replace++; //get rid of ~
			printf("line 99\n");
			char* adding = "$HOME";
			printf("line 101\n");
			char expanded[strlen(adding) + strlen(tok)];
			printf("line 104\n");
			sprintf(expanded, "%s%s", adding, tok+1);
			printf("line 106\n");
			strcpy(tok, expanded);
			printf("line 108\ntok is %s\n", tok);
		}
*/