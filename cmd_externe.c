#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include "cmd_externe.h"

int return_value = 0;
int command_not_found = 0;

int alloc_mem_for_args(char **arg, char **cmd) {
    for (int i = 0; i < MAX_ARGS_NUMBER; i++) {
        arg[i] = malloc(100*sizeof(char));
        if (!arg[i]) goto error_malloc;
    }
    arg[0] = cmd[0]; 
    return 0;

    error_malloc:
        perror("malloc");
        return 1;
}

void exec_cmd(char **arg) {
    if (execvp(arg[0], arg) == -1) { 
        printf("%s: command not found\n", arg[0]);
        command_not_found = 1;
    }
}

int fork_exec(char **arg) {
    int stat;
    pid_t pid = fork();
    switch (pid) {
        case -1:
            goto error_fork;
        case 0:
            exec_cmd(arg);
        default:
            wait(&stat);
            if (command_not_found == 1) { 
                return_value = 127;
                command_not_found = 0;
            }
            else {
                if (WIFEXITED(stat)) return_value = WEXITSTATUS(stat);
            }
            break;
    }
    return 0;

    error_fork:
        perror("fork");
        return 1;
}

int wildcard(int nb_args, char **arg) {
    char **other_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
    char **wildcard_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
    if (!other_arg || !wildcard_arg) goto error_malloc;
    
    alloc_mem_for_args(other_arg, arg);
    alloc_mem_for_args(wildcard_arg, arg);
            
    int i = 1;
    for (int l = 1; l < nb_args; l++) {
        if (strcmp(arg[l], "*") != 0) {
            other_arg[i] = arg[l];
            i++;
        }
    }
    other_arg[i] = NULL;

    if (i > 1) {
        fork_exec(other_arg);
    }

    DIR *dir = opendir(".");
    struct dirent *ent;

    int j = 1;
    while ((ent = readdir(dir))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

        char *filename = ent->d_name;

        // On teste si le premier caractère du nom du fichier est un point.
        if (filename[0] == '.') continue;

        wildcard_arg[j] = filename;
        j++;
    }
    closedir(dir);
    wildcard_arg[j] = NULL;

    fork_exec(wildcard_arg);

    free(other_arg);
    free(wildcard_arg);

    return 0;

    error_malloc:
        perror("malloc");
        return 1;
}

int handle_external_cmd(char *line) {
    int stat;
    pid_t pid = fork();
    switch (pid) {
        case -1:
            goto error_fork;
        case 0:
            char **arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
            if (!arg) goto error_malloc;

            for (int i = 0; i < MAX_ARGS_NUMBER; i++) {
                arg[i] = malloc(100*sizeof(char));
                if (!arg[i]) goto error_malloc;
            }

            int i = 0;
            char *token = strtok(line, " ");
            while (token != NULL) {
                arg[i] = token;
                i++;
                token = strtok(NULL, " ");
            }
            arg[i] = NULL;

            int wildcard_bool = 0;
            for (int j = 0; j < i; j++) {
                if (strchr(arg[j], '*') != NULL) {
                    wildcard_bool = 1;
                    wildcard(i, arg);
                }
            }
                    
            if (wildcard_bool == 0) {
                exec_cmd(arg);
            }

            free(arg);
            free(token);

            exit(0);
            default:
                wait(&stat);
                if (command_not_found == 1) { 
                    return_value = 127;
                    command_not_found = 0;
                }
                else {
                    if (WIFEXITED(stat)) return_value = WEXITSTATUS(stat);
                }
                break;
    }

    return return_value;

    error_malloc:
        perror("malloc");
        return 1;

    error_fork:
        perror("fork");
        return 1;
}