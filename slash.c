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
#include <fnmatch.h>
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
 * Interpreteur de commande qui permet d'exécuter des commandes internes et externes. 
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

        if (last_return_value == 0)
            color = GREEN;
        else
            color = RED;

        sprintf(prompt, "\001%s\002[%d]\001%s\002%s\001%s\002$ ", color, last_return_value, CYAN, cwd_prompt, DEFAULT);
        char *line = readline(prompt);
        
        if(!line) {
            free(line);
            exit(last_return_value);
        }

        if (strlen(line) > 0) add_history(line);
        if (strlen(line) == 0) continue;

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
            int stat;
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

                    int wildcard = 0;
                    // Gestion wildcard
                    for (int j = 1; j < i; j++) {
                        if (strchr(arg[j], '*') != NULL) {
                            // On a trouvé un wildcard
                            wildcard = 1;

                            // Si il y a d'autres arguments en plus du wildcard, on execute la commande sur les arguments correspondants.
                            char **other_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
                            if (other_arg == NULL) {
                                perror("malloc");
                                return 1;
                            }
                            for (int k = 0; k < MAX_ARGS_NUMBER; k++) {
                                other_arg[k] = malloc(100*sizeof(char));
                                if (other_arg[k] == NULL) {
                                    perror("malloc");
                                    return 1;
                                }
                            }
                            other_arg[0] = arg[0];
                            int k = 1;
                            for (int l = 1; l < i; l++) {
                                if (strcmp(arg[l], "*") != 0) {
                                    other_arg[k] = arg[l];
                                    k++;
                                }
                            }
                            other_arg[k] = NULL;

                            if (k > 1) {
                                int stat2;
                                pid_t pid2 = fork();
                                switch (pid2) {
                                    case -1:
                                        perror("fork");
                                        return 1;
                                    case 0:
                                        execvp(other_arg[0], other_arg);
                                    default:
                                        wait(&stat2);
                                        if (WIFEXITED(stat2)) last_return_value = WEXITSTATUS(stat2);
                                        break;
                                }
                            }

                            // On parcours ensuite le répertoire courant et on execute la commande sur les fichiers correspondants.
                            DIR *dir = opendir(".");
                            struct dirent *ent;
                            char **wildcard_arg = malloc(MAX_ARGS_NUMBER*sizeof(char*));
                            if (wildcard_arg == NULL) {
                                perror("malloc");
                                return 1;
                            }
                            for (int w = 0; w < MAX_ARGS_NUMBER; w++) {
                                wildcard_arg[w] = malloc(100*sizeof(char));
                                if (wildcard_arg[w] == NULL) {
                                    perror("malloc");
                                    return 1;
                                }
                            }
                            wildcard_arg[0] = arg[0];
                            int w = 1;
                            while ((ent = readdir(dir))) {
                                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

                                // On veut obtenir le nom du fichier
                                char *filename = ent->d_name;
                                wildcard_arg[w] = filename;
                                w++;
                            }
                            closedir(dir);
                            wildcard_arg[w] = NULL;

                            int stat3;
                            pid_t pid3 = fork();
                            switch (pid3) {
                                case -1:
                                    perror("fork");
                                    return 1;
                                case 0:
                                    execvp(wildcard_arg[0], wildcard_arg);
                                default:
                                    wait(&stat3);
                                    if (WIFEXITED(stat3)) last_return_value = WEXITSTATUS(stat3);
                                    break;
                            }
                            break;
                        }
                    }

                    
                    if (wildcard == 0)
                        execvp(arg[0], arg);
                    exit(0);
                default:
                    wait(&stat);
                    if (WIFEXITED(stat)) last_return_value = WEXITSTATUS(stat);
                    break;
            }
        }
        
        free(line_cpy);
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

