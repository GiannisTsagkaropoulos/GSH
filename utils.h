#ifndef UTILS_H
#define UTILS_H

#include <limits.h>

int dir_exists(const char *path);
int is_exec(char *fullpath, char *program);
void create_fullpath(char *target_dir, const char *cwd, const char *cd_arg);
int handle_home_dir(char *target_dir, const char *cd_arg);

#endif