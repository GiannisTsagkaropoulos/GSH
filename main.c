#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/errno.h>
#include "utils.h"
#include "builtins.h"

#define ARG_SEP " \t\r"
#define REDIR_OUT_1 ">"
#define REDIR_OUT_2 "1>"
#define APPEND_OUT_1 ">>"
#define APPEND_OUT_2 "1>>"
#define REDIR_ERR "2>"
#define APPEND_ERR "2>>"
#define FIRST_PATH_SEP
#define PROMPT "GSH"
#define SINGLE_QUOTE '\''
#define DOUBLE_QUOTE '\"'
#define BACKSLASH '\\'

/**
 * @brief Parse user input to program and arguments, respecting single quote.
 * Caller is responsible for freeing *out_input_cpy returned argv.
 *
 * @param input
 * @return * argv array
 */
char **parse_argv(char **out_input_cpy, int *out_argc, const char *input)
{
  char *input_cpy = strdup(input);
  if (!input_cpy)
    return NULL;

  input_cpy[strcspn(input_cpy, "\n\r")] = '\0';
  *out_input_cpy = input_cpy;

  int argc = 0;
  int max_args = 10;
  char **argv = malloc(max_args * sizeof(char *));

  bool in_quote = false;
  bool in_dquote = false;
  bool in_token = false;

  char *r_ptr = input_cpy;
  char *w_ptr = input_cpy;

  while (*r_ptr != '\0')
  {
    if (!in_quote && !in_dquote && isspace((unsigned char)*r_ptr)) // Unquoted space or tab
    {
      if (in_token)
      {
        *w_ptr++ = '\0';
        in_token = false;
      }
    }
    else if (*r_ptr == SINGLE_QUOTE && !in_dquote)
    {
      in_quote = !in_quote;

      if (!in_token)
      {
        argv[argc++] = w_ptr;
        in_token = true;

        if (argc >= max_args)
        {
          max_args *= 2;
          argv = realloc(argv, max_args * sizeof(char *));
        }
      }
    }
    else if (*r_ptr == DOUBLE_QUOTE)
    {
      in_dquote = !in_dquote;
      if (!in_token)
      {
        argv[argc++] = w_ptr;
        in_token = true;

        if (argc >= max_args)
        {
          max_args *= 2;
          argv = realloc(argv, max_args * sizeof(char *));
        }
      }
    }
    else if (*r_ptr == BACKSLASH && !in_quote)
    {
      if (!in_token)
      {
        argv[argc++] = w_ptr;
        in_token = true;
        if (argc >= max_args)
        {
          max_args *= 2;
          argv = realloc(argv, max_args * sizeof(char *));
        }
      }
      r_ptr++;
      *w_ptr++ = *r_ptr;
    }
    else if (*r_ptr == BACKSLASH && in_dquote)
    {
      if (!in_token)
      {
        argv[argc++] = w_ptr;
        in_token = true;
        if (argc >= max_args)
        {
          max_args *= 2;
          argv = realloc(argv, max_args * sizeof(char *));
        }
      }
      r_ptr++;
      *w_ptr++ = *r_ptr;
    }
    else // Normal character
    {
      if (!in_token)
      {
        argv[argc++] = w_ptr;
        in_token = true;

        if (argc >= max_args)
        {
          max_args *= 2;
          argv = realloc(argv, max_args * sizeof(char *));
        }
      }
      *w_ptr++ = *r_ptr;
    }
    r_ptr++;
  }

  if (in_token)
    *w_ptr = '\0';

  if (argc >= max_args)
    argv = realloc(argv, (argc + 1) * sizeof(char *));

  argv[argc] = NULL;
  *out_argc = argc;

  return argv;
}

void execute_command(const char *user_input)
{
  int argc = 0;
  char *input_cpy = NULL;
  char **argv = parse_argv(&input_cpy, &argc, user_input);

  if (!argv || argc == 0)
  {
    if (argv)
      free(argv);
    if (input_cpy)
      free(input_cpy);
    return;
  }

  int argc_cpy = 0;
  char **argv_cpy = malloc((argc + 1) * sizeof(char *));
  char *stdout_path = NULL;
  char *stderr_path = NULL;

  bool append_out = false;
  bool append_err = false;

  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], REDIR_OUT_1) == 0 || strcmp(argv[i], REDIR_OUT_2) == 0 || strcmp(argv[i], APPEND_OUT_1) == 0 || strcmp(argv[i], APPEND_OUT_2) == 0)
    {
      if (i + 1 >= argc)
      {
        printf("parse error near `\\n'\n");
        free(argv);
        free(argv_cpy);
        free(input_cpy);
        return;
      }
      if (strcmp(argv[i], APPEND_OUT_1) == 0 || strcmp(argv[i], APPEND_OUT_2) == 0)
      {
        append_out = true;
      }
      stdout_path = argv[i + 1];
      i++;
    }
    else if ((strcmp(argv[i], REDIR_ERR) == 0) || (strcmp(argv[i], APPEND_ERR) == 0))
    {
      if (i + 1 >= argc)
      {
        printf("parse error near `\\n'\n");
        free(argv);
        free(argv_cpy);
        free(input_cpy);
        return;
      }
      if (strcmp(argv[i], APPEND_ERR) == 0)
      {
        append_err = true;
      }
      stderr_path = argv[i + 1];
      i++;
    }
    else
    {
      argv_cpy[i] = argv[i];
      argc_cpy++;
    }
  }
  argv_cpy[argc_cpy] = NULL;

  FILE *stdout_stream = stdout;
  if (stdout_path != NULL)
  {
    char cwd[PATH_MAX];
    char target_dir[PATH_MAX] = {0};

    getcwd(cwd, PATH_MAX);
    create_fullpath(target_dir, cwd, stdout_path);

    stdout_stream = fopen(stdout_path, append_out ? "a" : "w");
    if (stdout_stream == NULL)
    {
      printf("Failed to write stdout to: %s\n%s\n", stdout_path, strerror(errno));

      free(argv);
      free(argv_cpy);
      free(input_cpy);
      return;
    }
  }

  FILE *stderr_stream = stderr;
  if (stderr_path != NULL)
  {
    char cwd[PATH_MAX];
    char target_dir[PATH_MAX] = {0};

    getcwd(cwd, PATH_MAX);
    create_fullpath(target_dir, cwd, stderr_path);

    stderr_stream = fopen(stderr_path, append_err ? "a" : "w");
    if (stderr_stream == NULL)
    {
      printf("Failed to write stderr to: %s\n%s\n", stderr_path, strerror(errno));

      free(argv);
      free(argv_cpy);
      free(input_cpy);
      return;
    }
  }

  int target_out = fileno(stdout_stream);
  int target_err = fileno(stderr_stream);

  char *cmd = argv[0];
  char fullpath[PATH_MAX];

  if (strcmp(cmd, "echo") == 0)
    builtin_echo(argc_cpy, argv_cpy, stdout_stream);

  else if (strcmp(cmd, "type") == 0)
    builtin_type(argc_cpy, argv_cpy, stdout_stream, stderr_stream);

  else if (strcmp(cmd, "pwd") == 0)
    builtin_pwd(argc_cpy, stdout_stream, stderr_stream);

  else if (strcmp(cmd, "cd") == 0)
    builtin_cd(argc_cpy, argv_cpy, stdout_stream, stderr_stream);

  else if (is_exec(fullpath, cmd))
    builtin_exec(fullpath, argv_cpy, stdout_stream, stderr_stream);
  else
    printf("%s: command not found\n", cmd);

  if (target_out != STDOUT_FILENO)
  {
    fclose(stdout_stream);
  }

  if (target_err != STDERR_FILENO)
  {
    fclose(stderr_stream);
  }

  free(argv);
  free(argv_cpy);
  free(input_cpy);
}

int main()
{
  setbuf(stdout, NULL);
  char user_input[PATH_MAX];
  char cwd[PATH_MAX];
  printf("---- Welcome to GSH :D ----\n\n");
  while (1)
  {
    getcwd(cwd, PATH_MAX);
    printf("%s $ ", cwd);
    fgets(user_input, sizeof(user_input), stdin);

    if (strncmp(user_input, "exit", 4) == 0)
      break;

    execute_command(user_input);
  }
  return 0;
}
