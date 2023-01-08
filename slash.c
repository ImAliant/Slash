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
#include <fcntl.h>

#include "cmd/cmd_interne/cmd_interne.h"
#include "cmd/cmd_externe/cmd_externe.h"

#define GREEN   "\033[32m"
#define RED     "\033[91m"
#define CYAN    "\033[34m"
#define DEFAULT "\033[00m"

#define REDIR "%[^>^<^>>^>|]"

int last_return_value;
char *cwd_prompt;

int saved_out;

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
    saved_out = dup(STDOUT_FILENO);
    
    print_prompt();
}

/*
 * Libère la mémoire allouée a la variable globale cwd_prompt.
 */
void end() {
    free(cwd_prompt);
}

int exec_with_redirection(char *prompt, char *line) {
    char *cmd = malloc(100*sizeof(char));
    if (cmd == NULL) {
        perror("malloc");
        return 1;
    }
    char *red = malloc(100*sizeof(char));
    if (red == NULL) {
        perror("malloc");
        return 1;
    }
    char *file = malloc(100*sizeof(char));
    if (file == NULL) {
        perror("malloc");
        return 1;
    }
    
    sscanf(line, REDIR, cmd);
    if (strcpy(red, line + strlen(cmd)) == NULL) {
        perror("strcpy");
        return 1;
    }
    red = strtok(red, " ");
    int len = strlen(red);
    cmd[strlen(cmd)-1] = '\0';
    if (strcmp(red, ">") != 0 && strcmp(red, "<") != 0 && strcmp(red, ">>") != 0 && strcmp(red, ">|") != 0) {
        fprintf(stderr, "ERREUR : mauvaise redirection\n");
        last_return_value = 1;
    }

    file = line + strlen(cmd) + len + 2;
    file[strlen(file)] = '\0';

    if (strcmp(">", red) == 0) {
        int fd1 = open(file, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd1 == -1) {
            perror("open");
            return 1;
        }
        fflush(stdout);
        if (dup2(fd1, 1) == -1) {
            perror("dup2");
            return 1;
        }
        if (close(fd1) == -1) {
            perror("close");
            return 1;
        }
    }
    else if (strcmp("<", red) == 0) {
        int fd2 = open(file, O_RDONLY);
        if (fd2 == -1) {
            perror("open");
            return 1;
        }
        fflush(stdout);
        if (dup2(fd2, 0) == -1) {
            perror("dup2");
            return 1;
        }
        if (close(fd2) == -1) {
            perror("close");
            return 1;
        }
    }
    else if (strcmp(">>", red) == 0) {
        int fd3 = open(file, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        if (fd3 == -1) {
            perror("open");
            return 1;
        }
        fflush(stdout);
        if (dup2(fd3, 1) == -1) {
            perror("dup2");
            return 1;
        }
        if (close(fd3) == -1) {
            perror("close");
            return 1;
        }
    }
    else if (strcmp(">|", red) == 0) {
        int fd4 = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd4 == -1) {
            perror("open");
            return 1;
        }
        fflush(stdout);
        if (dup2(fd4, 1) == -1) {
            perror("dup2");
            return 1;
        }
        if (close(fd4) == -1) {
            perror("close");
            return 1;
        }
    }

    // On execute la commande
    // On sépare la commande en tokens
    char *cmd_cpy = strdup(cmd);
    char *token = strtok(cmd_cpy, " ");
    char *cmd_interne[3] = {"cd", "pwd", "exit"};
    int bool_interne = 0;
    for (int i = 0; i < 3; i++) {
        if (strcmp(token, cmd_interne[i]) == 0) {
            bool_interne = 1;
            break;
        }
    }
    
    char *cmd_no_args = malloc(100*sizeof(char));
    if (cmd_no_args == NULL) {
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
    
    if (bool_interne == 1) {
        if (sscanf(cmd, "%s %s %s", cmd_no_args, arg, ref) == 3) {
            if (strcmp(cmd_no_args, "cd") == 0) {
                last_return_value = cmd_cd(arg, ref);
            }
        }
        else if (sscanf(cmd, "%s %s", cmd_no_args, arg) == 2) {
            if (strcmp(cmd_no_args, "pwd") == 0)
                last_return_value = cmd_pwd(arg);
            else if (strcmp(cmd_no_args, "cd") == 0) {
                last_return_value = cmd_cd("", arg);
            }
            else if (strcmp(cmd_no_args, "exit") == 0) {
                int val = atoi(arg);
                free(line);
                free(cmd);
                free(cmd_no_args);
                free(arg);
                free(ref);
                free(prompt);
                return cmd_exit(val);
            }
            else {
                fprintf(stderr, "Commande inconnue: %s\n", cmd_no_args);
                last_return_value = 127;
            }
        }
        else if (sscanf(cmd, "%s", cmd_no_args) == 1) {
            if (strcmp(cmd_no_args, "exit") == 0) return cmd_exit(last_return_value);
            else if (strcmp(cmd_no_args, "pwd") == 0) last_return_value = cmd_pwd("-L");
            else if (strcmp(cmd_no_args, "cd") == 0) last_return_value = cmd_cd("", "");
            else {
                fprintf(stderr, "Commande inconnue: %s\n", cmd_no_args);
                last_return_value = 127;
            }
        }
    }
    else {
        last_return_value = handle_external_cmd(cmd);
    }

    free(cmd);
    free(cmd_cpy);
    //free(red);
    //free(file);
    free(cmd_no_args);
    free(arg);
    free(ref);
    
    fflush(stdout);

    dup2(saved_out, STDOUT_FILENO);
    close(saved_out);

    return last_return_value;
}

int exec_without_redirection(char *prompt, char *line) {
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

    free(cmd);
    free(arg);
    free(ref);
    free(line_cpy);

    return last_return_value;
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
    rl_outstream = stderr;

    while(1) {
        
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

        if (strstr(line, ">") || strstr(line, "<") || strstr(line, ">>") || strstr(line, ">|")) {
            last_return_value = exec_with_redirection(prompt, line);
        }
        else {
            last_return_value = exec_without_redirection(prompt, line);
        }
        free(line);
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

