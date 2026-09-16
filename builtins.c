#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include "builtins.h"
#include "utils.h"

const char *VALID_COMMANDS[4] = {"exit", "echo", "type", "cd"};

void builtin_type(int argc, char **argv, FILE *stdout_stream, FILE *stderr_stream)
{
  if (argc < 2)
    return;
  char *cmd = argv[1];

  for (int i = 0; i < NUM_COMMANDS; i++)
  {
    if (strcmp(cmd, VALID_COMMANDS[i]) == 0)
    {
      fprintf(stdout_stream, "%s is a shell builtin\n", cmd);
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
      fprintf(stdout_stream, "%s is %s\n", cmd, fp);
      free(PATH);
      return;
    }
  }
  fprintf(stderr_stream, "%s not found\n", cmd);
  free(PATH);
}

void builtin_pwd(int argc, FILE *stdout_stream, FILE *stderr_stream)
{
  if (argc > 1)
  {
    fprintf(stderr_stream, "pwd: too many arguments\n");
    return;
  }

  char path[PATH_MAX];
  if (getcwd(path, PATH_MAX))
  {
    fprintf(stdout_stream, "%s\n", path);
  }
}

void builtin_cd(int argc, char **argv, FILE *stdout_stream, FILE *stderr_stream)
{
  if (argc > 2)
  {
    fprintf(stderr_stream, "cd: too many arguments\n");
    return;
  }

  char *cd_arg = (argc > 0) ? argv[1] : "~";

  char cwd[PATH_MAX];
  char target_dir[PATH_MAX] = {0};

  getcwd(cwd, PATH_MAX);
  create_fullpath(target_dir, cwd, cd_arg);

  if (!dir_exists(target_dir))
  {
    fprintf(stderr_stream, "cd: %s : No such file or directory\n", target_dir);
    return;
  }

  int res = chdir(target_dir);
  if (res == -1)
  {
    fprintf(stderr_stream, "cd failed: %s\n", strerror(errno));
    return;
  }
  fprintf(stdout_stream, "%s", target_dir);
}

void builtin_exec(char *fullpath, char **argv, FILE *stdout_stream, FILE *stderr_stream)
{
  pid_t pid = fork();
  if (pid == 0)
  {
    int target_out_fd = fileno(stdout_stream);
    if (target_out_fd != STDOUT_FILENO)
    {
      if (dup2(target_out_fd, STDOUT_FILENO) == -1)
      {
        fprintf(stderr_stream, "dup2 failed");
        exit(1);
      }
    }

    int target_err_fd = fileno(stderr_stream);
    if (target_err_fd != STDERR_FILENO)
    {
      if (dup2(target_err_fd, STDERR_FILENO) == -1)
      {
        fprintf(stderr_stream, "dup2 failed");
        exit(1);
      }
    }

    if (execv(fullpath, argv) == -1)
    {
      fprintf(stderr_stream, "execv failed");
      exit(1);
    }
  }
  else if (pid > 0)
  {
    int status;
    waitpid(pid, &status, 0);
  }
  else
  {
    fprintf(stderr_stream, "fork failed");
  }
}

void builtin_echo(int argc, char **argv, FILE *stdout_stream)
{
  for (int i = 1; i < argc; i++)
  {
    fprintf(stdout_stream, "%s%s", argv[i], i < argc - 1 ? " " : "");
  }
  fprintf(stdout_stream, "\n");
}
