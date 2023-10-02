#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char ** items;
    size_t size;
} tokenlist;

char * get_input(void);
tokenlist * get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void doCode(tokenlist *tokens);
char* tilde(char *token);
char * pathSearch (char * token);
bool isInternal(char* token);
void handleInternal(tokenlist *tokens);
void echo(tokenlist *tokens);
void handleExternal(tokenlist *tokens);
void cd(tokenlist *tokens);
