#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pwd.h>
#include "utils.h"

int dir_exists(const char *path)
{
  struct stat status;

  stat(path, &status);
  // remove bit mask for file type.  https://manpages.debian.org/testing/manpages/S_ISDIR.3.en.html
  if ((status.st_mode & S_IFMT) == S_IFDIR)
  {
    return 1;
  }
  return 0;
}

int is_exec(char *fullpath, char *program)
{
  if (!program)
    return 0;

  const char *path_env = getenv("PATH");
  if (!path_env)
    return 0;

  char *path_cpy = strdup(path_env);
  char *p = strtok(path_cpy, ":");
  int found = 0;

  for (; p != NULL; p = strtok(NULL, ":"))
  {
    snprintf(fullpath, PATH_MAX, "%s/%s", p, program);
    if (access(fullpath, X_OK) == 0)
    {
      found = 1;
      break;
    }
  }

  free(path_cpy);
  return found;
}

void create_fullpath(char *target_dir, char *cwd, char *cd_arg)
{
  if (cd_arg[0] == '~')
  {
    int res = handle_home_dir(target_dir, cd_arg);
    if (res != 0)
      return;
  }

  // Absolute path
  if (strncmp(cd_arg, "/", 1) == 0)
  {
    strncpy(target_dir, cd_arg, PATH_MAX);
    return;
  }

  // Relative path
  char relative_path[PATH_MAX];

  if (strncmp(cd_arg, "./", 2) == 0)
  {
    strcpy(relative_path, cd_arg + 1);
  }
  else if (isalnum(cd_arg[0]) || (!strncmp(cd_arg, ".", 1) && (strlen(cd_arg) > 1) && isalnum(cd_arg[2])))
  { // handle relative path which starts with hidden directory
    relative_path[0] = '/';
    strcpy(relative_path + 1, cd_arg);
  }

  if (!strncmp(relative_path, "/", 1))
  {
    snprintf(target_dir, PATH_MAX, "%s/%s", cwd, relative_path + 1);
    return;
  }

  // "starts with .."
  char *arg_token = strdup(cd_arg);
  arg_token = strtok(arg_token, "/ \t\r\n");
  int steps_back = 0;
  while (arg_token && !strcmp(arg_token, ".."))
  {
    if (strcmp(arg_token, "..") == 0)
    {
      steps_back++;
    }
    arg_token = strtok(NULL, "/");
  }

  char **path_arr = NULL;
  char *cwd_token = strtok(cwd, "/");
  int dir_num = 1;
  while (cwd_token != NULL)
  {
    path_arr = realloc(path_arr, sizeof(char *) * dir_num);
    path_arr[dir_num - 1] = cwd_token;
    cwd_token = strtok(NULL, "/");
    dir_num++;
  }
  dir_num -= 1;
  int max_path = dir_num;

  int diff = dir_num - steps_back;
  int idx = diff < 0 ? 0 : diff;
  while (arg_token != NULL)
  {
    if (idx > max_path)
    {
      path_arr = realloc(path_arr, sizeof(char *) * idx);
    }
    path_arr[idx] = arg_token;
    arg_token = strtok(NULL, "/");
    idx++;
  }

  size_t size = 0;
  char temp_dir[PATH_MAX] = "";

  for (int i = 0; i < idx; i++)
  {
    char next_dir[PATH_MAX];
    char *dir = path_arr[i];

    size_t dir_size = strlen(dir);
    size = size == 0 ? 2 + dir_size : size + 1 + dir_size;

    snprintf(next_dir, PATH_MAX, "%s/%s", temp_dir, path_arr[i]);
    strncpy(temp_dir, next_dir, PATH_MAX);
  }
  strncpy(target_dir, temp_dir, size);
  free(arg_token);
  return;
}

int handle_home_dir(char *target_dir, char *cd_arg)
{
  // Handle "~" or "~/"
  if (cd_arg[1] == '\0' || cd_arg[1] == '/')
  {
    const char *home = getenv("HOME");
    if (home == NULL)
    {
      fprintf(stderr, "cd: HOME not set\n");
      return 1;
    }
    sprintf(target_dir, "%s%s", home, cd_arg + 1);
    return 0;
  }
  // Handle "~username"
  else
  {
    char *slash = strchr(cd_arg, '/');
    if (slash)
      *slash = '\0';

    struct passwd *pw = getpwnam(cd_arg + 1);
    if (pw == NULL)
    {
      fprintf(stderr, "cd: no such user: %s\n", cd_arg + 1);
      return 1;
    }

    // Re-append the rest of the path if there was a slash
    if (slash)
    {
      *slash = '/';
      sprintf(target_dir, "%s%s", pw->pw_dir, slash);
    }
    else
    {
      strncpy(target_dir, pw->pw_dir, PATH_MAX);
    }
    return 0;
  }
}
