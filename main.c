#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    printf("%s: command not found\n", user_input);
  }

  return 0;
}