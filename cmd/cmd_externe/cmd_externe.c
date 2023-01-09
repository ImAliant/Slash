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

int find_and_exec_other_args(int nb_args, char **other_arg, char **arg) {
    int i = 1;
    for (int l = 1; l < nb_args; l++) {
        if (strcmp(arg[l], "*") != 0 && strstr(arg[l], "*.") == NULL) {
            other_arg[i] = arg[l];
            i++;
        }
    }
    other_arg[i] = NULL;

    if (i > 1) {
        fork_exec(other_arg);
    }

    return 0;
}

int wildcard_simple(int nb_args, char **arg) {
    char **other_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
    char **wildcard_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
    if (!other_arg || !wildcard_arg) goto error_malloc;
    
    alloc_mem_for_args(other_arg, arg);
    alloc_mem_for_args(wildcard_arg, arg);
            
    find_and_exec_other_args(nb_args, other_arg, arg);

    DIR *dir = opendir(".");
    struct dirent *ent;

    int j = 1;
    while ((ent = readdir(dir))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

        char *filename = ent->d_name;

        // Si le fichier est un fichier caché on n'execute pas de commande dessus.
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

int wildcard_extension(int nb_args, char **arg, char *first_occ) {
    char **other_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
    char **wildcard_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
    if (!other_arg || !wildcard_arg) goto error_malloc;

    alloc_mem_for_args(other_arg, arg);
    alloc_mem_for_args(wildcard_arg, arg);

    find_and_exec_other_args(nb_args, other_arg, arg);

    char *extension = first_occ + 2;
    DIR *dir = opendir(".");
    struct dirent *ent;

    int j = 1;
    while ((ent = readdir(dir))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        // On veut obtenir l'extension du fichier
        char *filename = ent->d_name;
        char *ext = strchr(filename, '.');
        if (ext != NULL) {
            ext++;
            if (strcmp(ext, extension) == 0) {
                // On a trouvé un fichier qui correspond à l'extension.
                wildcard_arg[j] = filename;
                j++;
            }
        }
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

int wildcard_path_extension(int nb_args, char **arg, char *first_occ) {
    char **other_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
    char **wildcard_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
    if (!other_arg || !wildcard_arg) goto error_malloc;

    alloc_mem_for_args(other_arg, arg);
    alloc_mem_for_args(wildcard_arg, arg);

    find_and_exec_other_args(nb_args, other_arg, arg);

    char *extension = first_occ + 3;
    
    // On doit chercher dans quel argument se trouve le chemin wildcard.
    int i = 0;
    while (arg[i] != NULL) {
        if (strstr(arg[i], "*.") != NULL) break;
        i++;
    }

    // On veut recuperer le chemin sans le wildcard.
    char *path = malloc(100*sizeof(char));
    if (!path) goto error_malloc;
    strcpy(path, arg[i]);
    char *last_occ = strrchr(path, '/');
    *last_occ = '\0';
    
    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("No such file or directory\n");
        return 1;
    }
    struct dirent *ent;

    int j = 1;
    while ((ent = readdir(dir))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        
        char *filename = ent->d_name;
        if (filename[0] == '.') continue;

        char *ext = strchr(filename, '.');
        if (ext != NULL) {
            ext++;
            if (strcmp(ext, extension) == 0) {
                wildcard_arg[j] = filename;
                j++;
            }
        }
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
    if (pid == -1) goto error_fork;
    if (pid == 0) {
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
            char *ptr;
            if ((ptr = strstr(arg[j], "/*.")) != NULL) {
                wildcard_bool = 1;
                wildcard_path_extension(i, arg, ptr);
            }
            else if ((ptr = strstr(arg[j], "*.")) != NULL) {
                wildcard_bool = 1;
                wildcard_extension(i, arg, ptr);
            }
            else if (strchr(arg[j], '*') != NULL) {
                wildcard_bool = 1;
                wildcard_simple(i, arg);
            }
        }
                
        if (wildcard_bool == 0) {
            exec_cmd(arg);
        }
        free(arg);
        free(token);
        exit(0);
    }
    else {
        waitpid(pid, &stat, 0);
        if (command_not_found == 1) { 
            return_value = 127;
            command_not_found = 0;
        }
        else {
            if (WIFEXITED(stat)) return_value = WEXITSTATUS(stat);
        }
    }

    return return_value;

    error_malloc:
        perror("malloc");
        return 1;

    error_fork:
        perror("fork");
        return 1;
}