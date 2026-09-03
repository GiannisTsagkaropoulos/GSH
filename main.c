#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

const int NUM_COMMANDS = 3;
const char *VALID_COMMANDS[3] = {"exit", "echo", "type"};

void handle_type(char *buf)
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

void handle_exec(char *fullpath, const char *input)
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
      printf("%s\n", user_input + 5);

    else if (strncmp(user_input, "type ", 5) == 0)
      handle_type(user_input + 5);

    else if (is_exec(fullpath, user_input))
      handle_exec(fullpath, user_input);

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

    return 0;
  }
}