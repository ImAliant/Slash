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
#include "cmd_externe.h"

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
            printf("exit\n");
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
        

   // Split the input line into separate commands at the pipeline characters
        char *commands[100];
        int num_commands = 0;
        char *command = strtok(line, "|");
        while (command != NULL) {
            commands[num_commands] = command;
            num_commands++;
            command = strtok(NULL, "|");
        }
        // Create a pipe for each pair of commands separated by a pipeline character
        int pipes[num_commands - 1][2];
        for (int i = 0; i < num_commands - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                exit(1);
            }
        }
        // Fork a child process for each command
        int pid[num_commands];
        for (int i = 0; i < num_commands; i++) {
            pid[i] = fork();
            if (pid[i] < 0) {
                perror("fork");
                exit(1);
            }
            if (pid[i] == 0) {
                            // Child process
                            // Redirect standard input or standard output to the read or write end of the appropriate pipe
                            if (i > 0) {
                                if (dup2(pipes[i-1][0], STDIN_FILENO) < 0) {
                                    perror("dup2");
                                    exit(1);
                                }
                                
                            } if (i < num_commands - 1) {
                                if (dup2(pipes[i][1], STDOUT_FILENO) < 0) {
                                    perror("dup2");
                                }
                            }
                       
                            //Execution des commandes : 
                            execvp(commands[i], commands[i]);
                            perror("execvp");
                            exit(1);
                }else {
                
 // Parent process
     
                



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
            last_return_value = handle_external_cmd(line);
        }
        
        free(line_cpy);
        free(line);
        free(cmd);
        free(arg);
        free(ref);
        free(prompt);

        print_prompt();
    }
        }
       // Close all the pipes
        for (int i = 0; i < num_commands - 1; i++) {
            if (close(pipes[i][0]) < 0) {
                perror("close");
                exit(1);
            }
            if (close(pipes[i][1]) < 0) {
                perror("close");
                exit(1);
            }
        }

                
        
// Wait for all child processes to finish
       int status;
        for (int i = 0; i < num_commands; i++) {
          
            if (waitpid(pid[i], &status, 0) < 0) {
                perror("waitpid");
                exit(1);
            }
            if (WIFEXITED(status)) {
                last_return_value = WEXITSTATUS(status);
            }
        }



        
    end();

    return last_return_value;
        
    }}

/*
 * Execute l'interpreteur de commande.
 */
int main(int argc, char *argv[]) {
    return slash();
}

