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
#include <readline/readline.h>
#include <readline/history.h>

#include "cmd_interne.h"

#define GREEN   "\033[32m"
#define RED     "\033[91m"
#define CYAN    "\033[34m"
#define DEFAULT "\033[00m"

int last_return_value;
char *cwd_prompt;

/*
 * Affiche le répertoire de travail courant.
 */
void print_prompt() {
    char *cwd = getenv("PWD");
    
    if (!cwd_prompt) {
        perror("malloc");
        cmd_exit(1);
    }

    if (strlen(cwd) > 26) 
        sprintf(cwd_prompt, "...%s", cwd + strlen(cwd) - 22);
    else
        sprintf(cwd_prompt, "%s", cwd);
}

/*
 * Initialise le prompt du shell et les variables globales.
 */
void init() {
    rl_initialize();
    
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    cwd_prompt = malloc(100*sizeof(char));
    if (!cwd_prompt) {
        perror("malloc");
        cmd_exit(1);
    }

    last_return_value = 0;
    
    print_prompt();
}

/*
 * Libère la mémoire allouée a la variable globale cwd_prompt.
 */
void end() {
    free(cwd_prompt);
}

/*
 * Interpreteur de commande qui permet d'exécuter des commandes internes. 
 * Les différentes commandes internes sont :
 *        - exit
 *        - cd
 *        - pwd
 * @return last_return_value : valeur de retour de la dernière commande exécutée ou la valeur attribué a la commande exit.
 */
int slash() {
    init();

    while(1) {
        rl_outstream = stderr;
        char *prompt = malloc(100*sizeof(char));
        if (prompt == NULL) {
            perror("malloc");
            return 1;
        }
        char *color;

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

        char *cmd = malloc(100*sizeof(char));
        if (cmd == NULL) {
            perror("malloc");
            return 1;
        }
        char *arg = malloc(100*sizeof(char));
        if (arg == NULL) {
            perror("malloc");
            return 1;
        }
        char *ref = malloc(100*sizeof(char));
        if (ref == NULL) {
            perror("malloc");
            return 1;
        }
        
        // On teste si il s'agit d'une commande interne.

        char *cmd_interne[] = {"exit", "cd", "pwd"};
        int bool_interne = 0;
        char *line_cpy = strdup(line);
        char *token = strtok(line_cpy, " ");
        for(int i=0; i<3; i++) {
            if (strcmp(token, cmd_interne[i]) == 0) {
                bool_interne = 1;
                break;
            }
        }
        

        if (bool_interne == 1) {
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

                    free(line);
                    free(cmd);
                    free(arg);
                    free(ref);
                    free(prompt);

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
        }
        else {
            pid_t pid = fork();
            switch (pid) {
                case -1:
                    perror("fork");
                    return 1;
                case 0:
                    char **arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
                    if (arg == NULL) {
                        perror("malloc");
                        return 1;
                    }
                    for (int i = 0; i < MAX_ARGS_NUMBER; i++) {
                        arg[i] = malloc(100*sizeof(char));
                        if (arg[i] == NULL) {
                            perror("malloc");
                            return 1;
                        }
                    }

                    int i = 0;
                    char *token = strtok(line, " ");
                    while (token != NULL) {
                        arg[i] = token;
                        i++;
                        token = strtok(NULL, " ");
                    }
                    arg[i] = NULL;

                    char *path = malloc(10*sizeof(arg[0]));
                    if (path == NULL) {
                        perror("malloc");
                        return 1;
                    }
                    sprintf(path, "/bin/%s", arg[0]);
                    last_return_value = execv(path, arg);
                    if (last_return_value == -1) {
                        perror("Commande inconnue");
                        return 127;
                    }
                default:
                    waitpid(pid, &last_return_value, 0);
                    break;
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

/*
 * Execute l'interpreteur de commande.
 */
int main(int argc, char *argv[]) {
    return slash();
}

