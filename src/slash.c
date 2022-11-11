#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS_NUMBER 4096
#define MAX_ARG_STRLEN 4096

int last_return_value = 0;

int cmd_pwd(char *arg) {
    if (strcmp(arg, "-L") == 0) {
        //TODO
        printf("TODO: -L\n");
        return 0;
    }
    else if (strcmp(arg, "-P") == 0) {
        char *cwd = getcwd(NULL, 0);
        if (cwd == NULL) {
            perror("getcwd");
            return -1;
        }
        printf("%s\n", cwd);
        return 0;
    }
    else {
        fprintf(stderr, "pwd: invalid option -- '%s'\n", arg);
        return -1;
    }
}

int slash() {
    while(1) {
        rl_outstream = stderr;
        char *prompt = malloc(30*sizeof(char));
        if (prompt == NULL) {
            perror("malloc");
            return -1;
        }

        snprintf(prompt, sizeof(prompt), "[%d]$ ", last_return_value);
        char *line = readline(prompt);
        if (strlen(line) > 0) add_history(line);

        char *cmd = malloc(30*sizeof(char));
        if (cmd == NULL) {
            perror("malloc");
            return -1;
        }
        char *arg = malloc(30*sizeof(char));
        if (arg == NULL) {
            perror("malloc");
            return -1;
        }
        
        if (sscanf(line, "%s %s", cmd, arg) == 2) {
            if (strcmp(cmd, "pwd") == 0)
                last_return_value = cmd_pwd(arg);
            else if (strcmp(cmd, "cd") == 0) {}
                //TODO CD
        }
        else if (sscanf(line, "%s", line) == 1) {
            if (strcmp(line, "exit") == 0) {
                //TODO EXIT
            }
            else if (strcmp(line, "pwd") == 0) last_return_value = cmd_pwd("-L");
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    slash();

    return EXIT_SUCCESS;
}