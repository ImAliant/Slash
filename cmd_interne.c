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

int cmd_exit(int val) {
    return val;
}

int cmd_cd(char *arg, char *ref) {
    char *cwd = malloc(MAX_ARG_STRLEN*sizeof(char));
    if (!cwd) {
        perror("malloc");
        return 1;
    }

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
            if (getenv("OLDPWD") == NULL) {
                fprintf(stderr, "cd: OLDPWD non defini\n");
                return 1;
            }
            
			chdir(getenv("OLDPWD"));
			strcpy(cwd,getenv("OLDPWD"));
		}
        else if (strcmp(ref, "..") == 0) {
			strcpy(cwd,getenv("PWD"));
			while (1){
				if (cwd[strlen(cwd)-1] == '/') {
					cwd[strlen(cwd)-1] = '\0' ;
					break;
			    }
			    else cwd[strlen(cwd)-1] = '\0'; 
		    }
		    chdir(cwd);
		}
        else{
			if (ref[0]!='/' && ref[0] != '.' && ref[1] != '.' ){
			    chdir(ref);
			    strcpy(cwd,getenv("PWD"));
			    strcat(cwd,"/");
			    strcat(cwd,ref);
		    }
		    else if ( ref[0] == '.' && ref[1] == '.' ) {
                char *path = malloc(strlen(ref)+1);
                if (!path) {
                    perror("malloc");
                    return 1;
                }

                char *backuppwd = getcwd(NULL,0);

				strcpy(path,ref);
				while (path[0]== '.' && path[1] == '.') {
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
                    memmove(path, path+3, strlen(path+3)+1);
			    }
			    if (chdir(path) != -1) {
                    strcat(cwd,path);
                }   
			    else {
					chdir(backuppwd);
					strcpy(cwd,backuppwd);
	            }
		        setenv("PWD",backuppwd,1);
                free(backuppwd);
                free(path);
		    }
		    else{
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
            getcwd(cwd,MAX_ARG_STRLEN);
            chdir(cwd);
		}
        else {
            char path[strlen(ref)];
            readlink(ref, path, sizeof(path));
            chdir(path);
            getcwd(cwd,MAX_ARG_STRLEN);
        }
    }

    code: 
    
    setenv("OLDPWD",getenv("PWD"),1);
    setenv("PWD",cwd,1);

    free(cwd);
    
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
        free(cwd);
        return 0;
    }
    else {
        fprintf(stderr, "pwd: invalid option -- '%s'\n", arg);
        return 1;
    }
}