#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include "utils.h"
#include "builtins.h"

#define ARG_SEP " \t\r"
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
  int argc;
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

  char *cmd = argv[0];
  char fullpath[PATH_MAX];

  if (strcmp(cmd, "echo") == 0)
    builtin_echo(argc, argv);

  else if (strcmp(cmd, "type") == 0)
    builtin_type(argc, argv);

  else if (strcmp(cmd, "pwd") == 0)
    builtin_pwd(argc);

  else if (strcmp(cmd, "cd") == 0)
    builtin_cd(argc, argv);

  else if (is_exec(fullpath, cmd))
    builtin_exec(fullpath, argv);
  else
    printf("%s: command not found\n", cmd);

  free(argv);
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
