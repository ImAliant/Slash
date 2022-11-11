#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096

int last_return_value = 0;
char* last_cwd ;
int cmd_exit() {
    return last_return_value;
}

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

int cmd_cd(char *arg,char* ref) {
    if(strcmp(arg,"-L")== 0 || strcmp(arg,"")){
        char *cwd = getcwd(NULL,0);
        if(cwd == NULL){
            perror("getcwd");
            return -1;
        }
        // Ne marche pas!!
    
        else if(strcmp(ref,"")==0){
            *cwd = chdir(getenv("HOME"));
           
            }
         if(strcmp(ref,"-")==0){
                
                strcpy(last_cwd,getcwd(NULL,0));
                *cwd = chdir(last_cwd);
                
            }

        else {
                *cwd = chdir(ref);
                
                }
                
    }
    



    return 0;
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
        char *ref = malloc(80*sizeof(char));
        if (ref == NULL) {
            perror("malloc");
            return -1;
        }
        
        if (sscanf(line, "%s %s", cmd, arg) == 2) {
            if (strcmp(cmd, "pwd") == 0)
                last_return_value = cmd_pwd(arg);
    
        if (sscanf(line, "%s %s %s", cmd, arg, ref) 
                                                == 3)
             if (strcmp(cmd, "cd") == 0)
                last_return_value = cmd_cd(arg,ref);
            }
        
        if (sscanf(line, "%s", line) == 1) {
            if (strcmp(line, "exit") == 0) {
                free(line);
                return cmd_exit();
            }
            else if (strcmp(line, "pwd") == 0) last_return_value = cmd_pwd("-L");
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    
    char* last_cwd = malloc(280*sizeof(char));
    if (last_cwd == NULL) {
      perror("malloc");
        return -1;
        }
    slash();

    return EXIT_SUCCESS;
}