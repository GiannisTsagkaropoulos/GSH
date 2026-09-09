#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
#include "builtins.h"
#include "utils.h"

const char *VALID_COMMANDS[4] = {"exit", "echo", "type", "cd"};

void builtin_type(int argc, char **argv)
{
  if (argc < 2)
    return;
  char *cmd = argv[1];

  for (int i = 0; i < NUM_COMMANDS; i++)
  {
    if (strcmp(cmd, VALID_COMMANDS[i]) == 0)
    {
      printf("%s is a shell builtin\n", cmd);
      return;
    }
  }

  char *path_env = getenv("PATH");
  if (!path_env)
    return;

  char *PATH = strdup(path_env);
  for (char *p = strtok(PATH, ":"); p != NULL; p = strtok(NULL, ":"))
  {
    char fp[PATH_MAX];
    snprintf(fp, PATH_MAX, "%s/%s", p, cmd);
    if (access(fp, X_OK) == 0)
    {
      printf("%s is %s\n", cmd, fp);
      free(PATH);
      return;
    }
  }
  printf("%s: not found\n", cmd);
  free(PATH);
}

void builtin_pwd(int argc)
{
  if (argc > 1)
  {
    printf("pwd: too many arguments\n");
    return;
  }

  char path[PATH_MAX];
  if (getcwd(path, PATH_MAX))
  {
    printf("%s\n", path);
  }
}

void builtin_cd(int argc, char **argv)
{
  if (argc > 2)
  {
    fprintf(stderr, "cd: too many arguments\n");
    return;
  }

  char *cd_arg = (argc > 0) ? argv[1] : "~";

  char cwd[PATH_MAX];
  char target_dir[PATH_MAX] = {0};

  getcwd(cwd, PATH_MAX);
  create_fullpath(target_dir, cwd, cd_arg);

  if (!dir_exists(target_dir))
  {
    printf("cd: %s : No such file or directory\n", target_dir);
    return;
  }

  int res = chdir(target_dir);
  if (res == -1)
  {
    printf("cd failed: %s\n", strerror(errno));
    return;
  }
}

void builtin_exec(char *fullpath, char **argv)
{

  pid_t pid = fork();
  if (pid == 0)
  {
    if (execv(fullpath, argv) == -1)
    {
      perror("execv failed");
      exit(1);
    }
  }
  else if (pid > 0)
  {
    int status;
    waitpid(pid, &status, 0);
  }
}

void builtin_echo(int argc, char **argv)
{
  for (int i = 1; i < argc; i++)
  {
    printf("%s%s", argv[i], i < argc - 1 ? " " : "");
  }
  printf("\n");
}