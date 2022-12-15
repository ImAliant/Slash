#ifndef _CMD_EXTERNE_H
#define _CMD_EXTERNE_H

#define MAX_ARGS_NUMBER 4096
#define MAX_ARG_STRLEN 4096

int alloc_mem_for_args(char **arg, char **cmd);

void exec_cmd(char **arg);

int fork_exec(char **arg);

int find_and_exec_other_args(int nb_args, char **other_arg, char **arg);

int wildcard_path_extension(int nb_args, char **arg, char *first_occ);

int wildcard_extension(int nb_args, char **arg, char *first_occ);

int wildcard_simple(int nb_args, char **arg);

int handle_external_cmd(char *line);

#endif