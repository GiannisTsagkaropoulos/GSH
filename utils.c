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

  // remove bit mask for file type.  https://manpages.debian.org/testing/manpages/S_ISDIR.3.en.html
  if (stat(path, &status) == 0 && (status.st_mode & S_IFMT) == S_IFDIR)
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

void create_fullpath(char *target_dir, const char *cwd, const char *cd_arg)
{
  if (cd_arg[0] == '~')
  {
    handle_home_dir(target_dir, cd_arg);
    return;
  }
  else if (cd_arg[0] == '/')
  {
    snprintf(target_dir, PATH_MAX, "%s", cd_arg);
    return;
  }

  char *path_stack[PATH_MAX / 2];
  int sp = 0;

  char *cwd_cpy = strdup(cwd);
  if (!cwd_cpy)
    return;

  char *token = strtok(cwd_cpy, "/");
  while (token != NULL)
  {
    if (strcmp(token, "..") == 0)
    {
      sp = sp > 0 ? sp - 1 : 0;
    }
    else if (strcmp(token, ".") != 0)
    {
      path_stack[sp++] = token;
    }
    token = strtok(NULL, "/");
  }

  char *cd_arg_cpy = strdup(cd_arg);
  if (!cd_arg_cpy)
  {
    free(cwd_cpy);
    return;
  }

  token = strtok(cd_arg_cpy, "/");
  while (token != NULL)
  {
    if (strcmp(token, "..") == 0)
    {
      sp = sp > 0 ? sp - 1 : 0;
    }
    else if (strcmp(token, ".") != 0)
    {
      path_stack[sp++] = token;
    }
    token = strtok(NULL, "/");
  }

  //
  char raw_path[PATH_MAX] = "";
  if (sp == 0)
  {
    strncpy(target_dir, "/", PATH_MAX);
  }
  else
  {
    size_t cur_len = 0;
    for (int i = 0; i < sp; i++)
    {
      // Write directly to the remaining buffer space
      char *to_add = path_stack[i];
      snprintf(raw_path + cur_len, PATH_MAX - cur_len, "/%s", to_add);
      cur_len += 1 + strlen(to_add);
    }
    strncpy(target_dir, raw_path, PATH_MAX);
  }

  free(cwd_cpy);
  free(cd_arg_cpy);
}

int handle_home_dir(char *target_dir, const char *cd_arg)
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
    sprintf(target_dir, "%s", home);
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
