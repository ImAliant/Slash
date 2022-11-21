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

#define MAX_ARGS_NUMBER 4096
#define MAX_ARG_STRLEN 4096

#define GREEN "\033[32m"
#define RED "\033[91m"
#define CYAN "\033[34m"
#define DEFAULT "\033[00m"

int last_return_value = 0;

char cwd[MAX_ARG_STRLEN];

char cwd_prompt[26];

void print_prompt() {
    char *cwd = getenv("PWD");
    if (strlen(cwd) > 26) {
        snprintf(cwd_prompt, sizeof(cwd_prompt), "...%s", cwd + strlen(cwd) - 22);
    }
    else {
        snprintf(cwd_prompt, sizeof(cwd_prompt), "%s", cwd);
    }
}

int cmd_exit(char *val) {
    int exit_value = atoi(val);

    if (exit_value == -10000) {
        return last_return_value;
    }
    else {
        return exit_value;
    }
}

int cmd_cd(char *arg, char *ref) {
	
	
    if (strcmp(ref, "") == 0) {
        chdir(getenv("HOME"));
        strcpy(cwd,getenv("HOME"));
       
        goto code;
    }
   

    struct stat st;
    stat(ref, &st);
    DIR* dir = opendir(ref);
    if( (strcmp(ref, "-")) != 0 && (ENOENT == errno)) {
		fprintf(stderr, "cd: %s: No such file or directory\n", ref);
        return 1;
    }
    closedir(dir);
    /*if (strcmp(ref, "-") != 0 && ( stat(ref, &st) == -1) ) {
		  fprintf(stderr, "%s\n", getcwd(NULL,0));
          fprintf(stderr, "cd: %s: No such file or directory\n", ref);
          return 1;
    } */
    
    if (strcmp(ref, "-") != 0) {
        if (S_ISDIR(st.st_mode)) {
            if (chdir(ref) == -1) {
                fprintf(stderr, "cd: %s: Permission denied\n", ref);
                return 1;
            }
        }
        else if (S_ISLNK(st.st_mode)) {
            if (chdir(ref) == -1) {
                fprintf(stderr, "cd: %s: Permission denied\n", ref);
                return 1;
            }
        }  
        else {
            fprintf(stderr, "cd: %s: Not a directory\n", ref);
            return 1;
        }
    }

    if (strcmp(arg, "-L") == 0 || strcmp(arg, "") == 0) {
        if (strcmp(ref, "-") == 0) {
			chdir(getenv("OLDPWD"));
			strcpy(cwd,getenv("OLDPWD"));
		}
        else if (strcmp(ref, "..") == 0) {
			//chdir("..");
			strcpy(cwd,getenv("PWD"));
			while (1){
				if (cwd[strlen(cwd)-1] == '/') {
					cwd[strlen(cwd)-1] = '\0' ;
					break;
			    }
			    else cwd[strlen(cwd)-1] = '\0'; 
		    }
		    chdir(cwd);
		    //setenv("PWD",cwd,1);
		}
        else{
			if (ref[0]!='/' && ref[0] != '.' && ref[1] != '.' ){
			 chdir(ref);
			 strcpy(cwd,getenv("PWD"));
			 strcat(cwd,"/");
			 strcat(cwd,ref);
		     }
		     else if ( ref[0] == '.' && ref[1] == '.' ) {
				 /////
				 char *path=(char *)malloc(MAX_ARG_STRLEN);
				 char *backuppwd=(char *)malloc(MAX_ARG_STRLEN);
				 strcpy(backuppwd,getcwd(NULL,0));
				 strcpy(path,ref);
				 while ( path[0]== '.' && path[1] == '.' ) {
					 strcpy(cwd,getenv("PWD"));
			         while (1){
				       if (cwd[strlen(cwd)-1] == '/') {
					   cwd[strlen(cwd)-1] = '\0' ;
					   break;
			           }
			           else cwd[strlen(cwd)-1] = '\0'; 
		             }
		             chdir(cwd);
		             setenv("PWD",cwd,1);
					 path=path+3;
			     }
			     if ( chdir(path) != -1 ) strcat(cwd,path);
			     else {
					 chdir(backuppwd);
					 strcpy(cwd,backuppwd);
		     }
		    chdir(backuppwd);
		    setenv("PWD",backuppwd,1);
					 
			 }
				    
		     else{
				 //if ( ref[strlen(ref)-1] == '/' ) ref[strlen(ref)-1] = '\0';
				 strcpy(cwd,ref);
				 chdir(cwd);
			 }
		   }
    }
    else if (strcmp(arg, "-P") == 0) {
       if (strcmp(ref, "-") == 0) {
            char path[strlen(getenv("OLDPWD"))];
            realpath(ref,path);
            chdir(path);
        }
        else if (strcmp(ref, "..") == 0) {
			strcpy(cwd,getcwd(NULL,0));
		    chdir(cwd);
		}
        else {
            char path[strlen(ref)];
            realpath(ref,path);
            chdir(path);
        }
    }

    code: 
    
    /*getcwd(cwd, sizeof(cwd));
        
    if (cwd == NULL) {
        perror("getcwd");
        return 1;
    }*/
    
    setenv("OLDPWD",getenv("PWD"),1);
    setenv("PWD",cwd,1);
    //char path[MAX_ARG_STRLEN];
    //setenv("CWD",realpath(cwd,path),1);

    print_prompt();
    
    return 0;
}


int cmd_pwd(char *arg) {
    if (strcmp(arg, "-L") == 0 ||strcmp(arg, "") == 0  ) {
		printf("%s\n", getenv("PWD"));
        return 0;
    }

    else if (strcmp(arg, "-P") == 0) {
        char *cwd = getcwd(NULL, 0);
        if (cwd == NULL) {
            perror("getcwd");
            return 1;
        }
        printf("%s\n", cwd);
        return 0;
    }
    else {
        fprintf(stderr, "pwd: invalid option -- '%s'\n", arg);
        return 1;
    }
}

int slash() {
    while(1) {
        rl_outstream = stderr;
        char *prompt = malloc(55*sizeof(char));
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


        snprintf(prompt, 55*sizeof(char), "\001%s\002[%d]\001%s\002%s\001%s\002$ ", color, last_return_value, CYAN, cwd_prompt, DEFAULT);
        char *line = readline(prompt);
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
                return cmd_exit(arg);
            }
            else {
                fprintf(stderr, "Commande inconnue: %s\n", cmd);
                last_return_value = 127;
            }
        }
        else if (sscanf(line, "%s", cmd) == 1) {
            if (strcmp(cmd, "exit") == 0) return cmd_exit("-10000");
            else if (strcmp(cmd, "pwd") == 0) last_return_value = cmd_pwd("-L");
            else if (strcmp(cmd, "cd") == 0) last_return_value = cmd_cd("", "");
            else {
                fprintf(stderr, "Commande inconnue: %s\n", cmd);
                last_return_value = 127;
            }
        }

    }

    return last_return_value;
}

int main(int argc, char *argv[]) {
    print_prompt();
   
    return slash();
}
