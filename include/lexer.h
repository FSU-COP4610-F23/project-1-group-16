#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char ** items;
    size_t size;
} tokenlist;

typedef struct{
    char commands[3][200];
    int count;
} commandHistory;

char * get_input(void);
tokenlist * get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void doCode(tokenlist *tokens);
void tilde(tokenlist *tokens);
char * pathSearch (char * token);
int isInternal(tokenlist *tokens);
int handleInternal(tokenlist *tokens);
void echo(tokenlist *tokens);
int handleExternal(tokenlist *tokens, bool inRedirection, bool outRedirection, char *out_file);
int cd(tokenlist *tokens);
void exitCommand();
void addCommandToValid(commandHistory *history, char *command);
void displayLastThree(commandHistory *history);
bool doOutRedirection(tokenlist *tokens, char *out_file);