#ifndef BUILTINS_H
#define BUILTINS_H

#define NUM_COMMANDS 4
extern const char *VALID_COMMANDS[4];

void builtin_type(int argc, char **argv, FILE *stdout_stream);
void builtin_pwd(int argc, FILE *stdout_stream);
void builtin_cd(int argc, char **argv, FILE *stdout_stream);
void builtin_exec(char *fullpath, char **argv, FILE *stdout_stream);
void builtin_echo(int argc, char **argv, FILE *stdout_stream);

#endif