#ifndef __MINI_SHELL_H__
#define __MINI_SHELL_H__

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stack>

// maximum length of user command
#define MAX_LEN 200 
// maximum number of args in user command
#define ARGS_LEN 64 

// represents user command
typedef struct _Command {
    // commands as it is
    char msg[MAX_LEN];
    // number of args in parsed command
    int argc;
    // parsed args
    char *argv[ARGS_LEN];
} Command;

void greeting();
void tokenize(char *msg, Command *command);
void cd_handler(Command *command, bool bg);
bool is_bg(Command *command);
int is_redirect(Command *command);
void redirect_handler(int redirect, Command *command);

#endif
