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
        if (strcmp(ref, "-") == 0) {                        // Accède à la référence précédente grâce à la variable d'environnement
            if (getenv("OLDPWD") == NULL) {                 // OLDPWD si elle existe dans le cas de cd /*-L*/ -
                fprintf(stderr, "cd: OLDPWD non defini\n");
                return 1;
            }
            
			chdir(getenv("OLDPWD"));
			strcpy(cwd,getenv("OLDPWD"));
		}
        else if (strcmp(ref, "..") == 0) {
			strcpy(cwd,getenv("PWD"));                    // Comme PWD n'a pas changé pour l'instant, il suffit de la prendre
			while (1){                                    // puis enlever les charactères jusqu'au prochain '/' pour revenir dans le parent.
				if (cwd[strlen(cwd)-1] == '/') {          // Le while remplace les derniers charactère par '\0' jusqu'au prochain '/'.
					cwd[strlen(cwd)-1] = '\0' ;
					break;
			    }
			    else cwd[strlen(cwd)-1] = '\0'; 
		    }
		    chdir(cwd);
		}
        else{
			if (ref[0]!='/' && ref[0] != '.' && ref[1] != '.' ){   // Cas ou on accède pas au dossier antérieur,
			    chdir(ref);                                        // on met à jour la variable cwd avec le PWD antérieur
			    strcpy(cwd,getenv("PWD"));                         // suivi de la référence du dossier accéder.
			    strcat(cwd,"/");
			    strcat(cwd,ref);
		    }
		    else if ( ref[0] == '.' && ref[1] == '.' ) {           // Cas ou on accède à un dossier antérieur autre que ".."
                char *path = malloc(strlen(ref)+1);                // (exemple ../../toto ). Une variable path est alloué pour 
                if (!path) {                                       // pouvoir savoir combien de fois "../" est présent dans la référence 
                    perror("malloc");
                    return 1;
                }

                char *backuppwd = getcwd(NULL,0);                 // Dans le cas ou le dosier n'existe pas, ou reviens à la référence absolue
                                                                  // du répertoire courant.
				strcpy(path,ref);
				while (path[0]== '.' && path[1] == '.') {         // Tant que path à des ../ dans sa référence, elle retourne au répertoire 
					strcpy(cwd,getenv("PWD"));                    // précédent. On met a jour la variable cwd pour y ajoute la référence path
			        while (1){                                    // si accesible.
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
			    
			    if (chdir(path) != -1) {                     // Si après la référence sans les "../" est accesible, on y accéde et on met
                    strcat(cwd,path);                        // à jour le dossier courant en rajoutant la référence path tronquée.
                }   
			    else {
					chdir(backuppwd);                        // Sinon, on accéde à la référence physique absolue de l'ancienne référence
					strcpy(cwd,backuppwd);                   // ou on était.
	            }
		        setenv("PWD",backuppwd,1);
                free(backuppwd);
                free(path);
		    }
		    else{
				strcpy(cwd,ref);                             // Dans le cas on donne la référence absolue, on copie simplement la référence
				chdir(cwd);                                  // du répertoire où on veut accéder dans cwd. 
			}
		}
    }
    else if (strcmp(arg, "-P") == 0) {
       if (strcmp(ref, "-") == 0) {
            char path[strlen(getenv("OLDPWD"))];    // Même principe que cd -L sauf que realpath fait en sorte qu'on accéde à la référence
            realpath(ref,path);                     // physique si le référence donnée est logique.
            chdir(path);
        }
        else if (strcmp(ref, "..") == 0) {          
            getcwd(cwd,MAX_ARG_STRLEN);
            chdir(cwd);
		}
        else {
            char path[strlen(ref)];                // Dans le cas normal, readlink permet de dire la référence physique de la référence 
            readlink(ref, path, sizeof(path));     // ou on veut accéder.
            chdir(path);                           // Puis on accède à la référence donnée par readlink.
            getcwd(cwd,MAX_ARG_STRLEN);  
        }
    }

    code: 
    
    setenv("OLDPWD",getenv("PWD"),1);              // On met à jour les environnement ( OLDPWD prend l'ancienne référence de PWD , 
    setenv("PWD",cwd,1);                           // et PWD est update à la référence logique du répertoire courant.

    free(cwd);
    
    return 0;
}



int cmd_pwd(char *arg) {
    if (strcmp(arg, "-L") == 0 ||strcmp(arg, "") == 0  ) {  // Prends l'environnement PWD pour trouver la référence absolue logique dans 
		printf("%s\n", getenv("PWD"));                      // le cas de pwd -L ou sans argument.
        return 0;
    }

    else if (strcmp(arg, "-P") == 0) {  // Renvoie la référence absolue physique grâce à la variable d'environnement CWD
        char *cwd = getcwd(NULL, 0);    // dans le cas de pwd -P.
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
