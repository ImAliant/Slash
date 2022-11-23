#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "cmd_interne.h"

#define GREEN   "\033[32m"
#define RED     "\033[91m"
#define CYAN    "\033[34m"
#define DEFAULT "\033[00m"

int last_return_value = 0;
char *cwd_prompt;

int print_prompt() {
    char *cwd = getenv("PWD");
    
    cwd_prompt = malloc(30*sizeof(char));
    if (!cwd_prompt) {
        perror("malloc");
        return 1;
    }

    if (strlen(cwd) > 26) 
        sprintf(cwd_prompt, "...%s", cwd + strlen(cwd) - 22);
    else
        sprintf(cwd_prompt, "%s", cwd);
}

int init() {
    rl_initialize();
    print_prompt();
}

void end() {
    free(cwd_prompt);
}

int slash() {
    init();

    while(1) {
        rl_outstream = stderr;
        char *prompt = malloc(45*sizeof(char));
        if (prompt == NULL) {
            perror("malloc");
            return 1;
        }
        char *color = malloc(10*sizeof(char));
        if (color == NULL) {
            perror("malloc");
            return 1;
        }

        if (last_return_value == 1 || last_return_value == 127)
            color = RED;
        else
            color = GREEN;

        sprintf(prompt, "\001%s\002[%d]\001%s\002%s\001%s\002$ ", color, last_return_value, CYAN, cwd_prompt, DEFAULT);
        char *line = readline(prompt);
        
        if(!line) {
            free(line);
            exit(last_return_value);
        }

        if (strlen(line) > 0) add_history(line);

        char *cmd = malloc(30*sizeof(char));
        if (cmd == NULL) {
            perror("malloc");
            return 1;
        }
        char *arg = malloc(30*sizeof(char));
        if (arg == NULL) {
            perror("malloc");
            return 1;
        }
        char *ref = malloc(30*sizeof(char));
        if (ref == NULL) {
            perror("malloc");
            return 1;
        }
        
        if (sscanf(line, "%s %s %s", cmd, arg, ref) == 3) {
            if (strcmp(cmd, "cd") == 0) {
                last_return_value = cmd_cd(arg, ref);
            }
        }
        else if (sscanf(line, "%s %s", cmd, arg) == 2) {
            if (strcmp(cmd, "pwd") == 0)
                last_return_value = cmd_pwd(arg);
            else if (strcmp(cmd, "cd") == 0) {
                last_return_value = cmd_cd("", arg);
            }
            else if (strcmp(cmd, "exit") == 0) {
                int val = atoi(arg);
                return cmd_exit(val);
            }
            else {
                fprintf(stderr, "Commande inconnue: %s\n", cmd);
                last_return_value = 127;
            }
        }
        else if (sscanf(line, "%s", cmd) == 1) {
            if (strcmp(cmd, "exit") == 0) return cmd_exit(last_return_value);
            else if (strcmp(cmd, "pwd") == 0) last_return_value = cmd_pwd("-L");
            else if (strcmp(cmd, "cd") == 0) last_return_value = cmd_cd("", "");
            else {
                fprintf(stderr, "Commande inconnue: %s\n", cmd);
                last_return_value = 127;
            }
        }
        free(line);
        free(cmd);
        free(arg);
        free(ref);
        free(prompt);

        print_prompt();
    }
    end();

    return last_return_value;
}

int main(int argc, char *argv[]) {
    return slash();
}
