#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

int main(int argc, char *argv[])
{
  setbuf(stdout, NULL);
  char user_input[1024];

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

    printf("%s: command not found\n", user_input);
  }

  return 0;
}