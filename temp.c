// REDIRECTION
        /* Format si redirection : la commande la redirection le fichier (%s %s %s) */
        /* la commande a des arguments qu'il faudra séparer */
        /* la redirection : >, <, >>, >| */
        /* le fichier : le nom du fichier */

        if (strlen(line) > 0) add_history(line);
        if (strlen(line) == 0) continue;

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
    
        if (strstr(line, ">") || strstr(line, "<") || strstr(line, ">>") || strstr(line, ">|")) {
            sscanf(line, REDIR, cmd);
            
            strcpy(red, line + strlen(cmd));
            red = strtok(red, " ");

            int len = strlen(red);
            cmd[strlen(cmd)-1] = '\0';

            // Si red est different de {>, <, >>, >|} alors erreur
            if (strcmp(red, ">") != 0 && strcmp(red, "<") != 0 && strcmp(red, ">>") != 0 && strcmp(red, ">|") != 0) {
                fprintf(stderr, "ERREUR : mauvaise redirection\n");
                continue;
            }
            // On recupere le fichier
            file = line + strlen(cmd) + len + 2;
            file[strlen(file)] = '\0';
            fprintf(stderr, "REDIR, cmd : %s, red : %s, file : %s,\n", cmd, red, file);
            if (strcmp(">", red) == 0) {
                int fd1 = open(file, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
                if (fd1 == -1) {
                    perror("open");
                    return 1;
                }
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
                if (dup2(fd4, 1) == -1) {
                    perror("dup2");
                    return 1;
                }
                if (close(fd4) == -1) {
                    perror("close");
                    return 1;
                }
            }

            last_return_value = exec_with_redirection(prompt, line, cmd);
            
            fflush(stdout);

            dup2(saved_out, STDOUT_FILENO);
            close(saved_out);
        }
        // PAS DE REDIRECTION
        else {
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
        }