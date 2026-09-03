#ifndef BUILTINS_H
#define BUILTINS_H

#define NUM_COMMANDS 4
extern const char *VALID_COMMANDS[4];

void builtin_type(int argc, char **argv);
void builtin_pwd(int argc);
void builtin_cd(int argc, char **argv);
void builtin_exec(char *fullpath, char **argv);
void builtin_echo(int argc, char **argv);

#endif