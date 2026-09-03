#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include <uuid/uuid.h>
#include <stdbool.h>

#define ARG_SEP " \t\r"
#define FIRST_PATH_SEP
#define PROMPT "GSH"
const int NUM_COMMANDS = 4;
const char *VALID_COMMANDS[4] = {"exit", "echo", "type", "cd"};

void builtin_type(char *buf)
{

  for (int i = 0; i < NUM_COMMANDS; i++)
  {
    if (strcmp(buf, VALID_COMMANDS[i]) == 0)
    {
      printf("%s is a shell builtin\n", buf);
      return;
    }
  }

  char *PATH = strdup(getenv("PATH"));

  for (char *p = strtok(PATH, ":"); p != NULL; p = strtok(NULL, ":"))
  {
    char fp[1024];
    sprintf(fp, "%s/%s", p, buf);
    if (access(fp, X_OK) == 0)
    {
      printf("%s is %s\n", buf, fp);
      return;
    }
  }
  printf("%s: not found\n", buf);
}

int is_exec(char *fullpath, const char *input)
{
  if (!input)
    return 0;

  const char *path_env = getenv("PATH");
  if (!path_env)
    return 0;

  char *path_cpy = strdup(path_env);
  char *input_cpy = strdup(input);

  if (!input_cpy || !path_cpy)
  {
    free(path_cpy);
    free(input_cpy);
    return 0;
  }

  char *program = strtok(input_cpy, " \t\n\r");
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
  free(input_cpy);
  return found;
}

void builtin_pwd(const char *input)
{
  char *path_sep = " \t\r";
  char *input_cpy = strdup(input);

  char *words = strtok(input_cpy, path_sep);
  if (strtok(NULL, path_sep) != NULL) // pwd is followed by arguments
  {
    printf("pwd: too many arguments\n");
    free(input_cpy);
    return;
  }

  char path[PATH_MAX];

  getcwd(path, PATH_MAX);
  printf("%s\n", path);

  free(input_cpy);
}

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

void create_fullpath(char *target_dir, char *cwd, char *cd_arg)
{
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
  for (int i = 0; i < idx; i++)
  {
    char *dir = path_arr[i];
    size_t dir_size = strlen(dir);
    size = size == 0 ? 2 + dir_size : size + 1 + dir_size;
    snprintf(target_dir, size, "%s/%s", target_dir, dir);
  }
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
      strncpy(target_dir, pw->pw_dir, sizeof(target_dir) - 1);
    }
    return 0;
  }
}

void builtin_cd(const char *input)
{
  char *input_cpy = strdup(input);
  if (!input_cpy)
  {
    printf("cd failed: %s\n", strerror(errno));
    return;
  }

  strtok(input_cpy, ARG_SEP);

  char *cd_arg = strtok(NULL, ARG_SEP);
  char *extra_arg = strtok(NULL, ARG_SEP);

  if (cd_arg == NULL)
    cd_arg = "~";

  if (extra_arg != NULL)
  {
    fprintf(stderr, "cd: too many arguments\n");
    free(input_cpy);
    return;
  }

  char cwd[PATH_MAX];
  char target_dir[PATH_MAX] = {0};

  if (cd_arg[0] == '~')
  {
    int res = handle_home_dir(target_dir, cd_arg);
    if (res != 0)
      return;
  }
  else
  {
    getcwd(cwd, PATH_MAX);
    create_fullpath(target_dir, cwd, cd_arg);
  }

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

void builtin_exec(char *fullpath, const char *input)
{
  char *input_cpy = strdup(input);
  if (!input_cpy)
    return;

  char *program = strtok(input_cpy, " \t\n\r");
  int argc = 1;
  char **argv = NULL;
  argv = realloc(argv, sizeof(char *) * argc);
  argv[argc - 1] = program;
  argc++;

  char *arg = strtok(NULL, " \t\n\r");
  while (arg != NULL)
  {
    argv = realloc(argv, sizeof(char *) * argc);
    argv[argc - 1] = arg;
    arg = strtok(NULL, " \t\n\r");
    argc++;
  }

  argv = realloc(argv, sizeof(char *) * argc);
  argv[argc - 1] = NULL;

  pid_t pid = fork();
  if (pid == 0)
  {
    if (execv(fullpath, argv) == -1)
    {
      printf("ERROR!\n");
      exit(1);
    }
  }
  else if (pid > 0)
  {
    int status;
    waitpid(pid, &status, 0);
  }

  free(input_cpy);
  free(argv);
}

void builtin_echo(char *arg)
{
  printf("%s", arg);
  return;
}

int main(int argc, char *argv[])
{
  setbuf(stdout, NULL);
  char user_input[1024];
  char fullpath[PATH_MAX];

  while (1)
  {
    printf("---- Welcome to GSH :D ----\n");
    printf("$ ");
    fgets(user_input, sizeof(user_input), stdin);
    user_input[strcspn(user_input, "\n")] = '\0';

    if (strncmp(user_input, "exit", 4) == 0)
      break;

    else if (strncmp(user_input, "echo ", 5) == 0)
      builtin_echo(user_input + 5);

    else if (strncmp(user_input, "type ", 5) == 0)
      builtin_type(user_input + 5);

    else if (strncmp(user_input, "pwd", 3) == 0)
      builtin_pwd(user_input);

    else if (strncmp(user_input, "cd", 2) == 0)
      builtin_cd(user_input);

    else if (is_exec(fullpath, user_input))
      builtin_exec(fullpath, user_input);

    else
    {
      char *input_cpy = strdup(user_input);
      char *program = strtok(input_cpy, " \t\n\r");
      if (program)
      {
        printf("%s: command not found\n", program);
      }
      free(input_cpy);
    }
  }
  return 0;
}