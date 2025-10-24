/*
 * This code implements a simple shell program
 * It supports the internal shell command "exit", 
 * backgrounding processes with "&", input redirection
 * with "<" and output redirection with ">".
 * However, this is not complete.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

extern char **getaline();

/*
 * Handle exit signals from child processes
 */
void sig_handler(int signal)
{
  int status;
  pid_t child_id;
  int result = waitpid(child_id, &status, 0);
  ;

  printf("Wait returned %d\n", result);
}

/*
 * The main shell function
 */
main()
{
  int i;
  char **args;
  int pipes;
  int marks;
  int exitCode = 0;

  // Loop forever
  while (1)
  {

    // Print out the prompt and get the input
    printf("->");
    args = getaline();

    pipes = 0;
    marks = 0;
    //debug code
    // printf("***********************\n");
    // int i;
    // for (i = 0; args[i] != NULL; i++)
    // {
    //   printf("Argument %d: %s\n", i, args[i]);
    // }
    // printf("***********************\n");
    for (i = 0; args[i + 1] != NULL; i++)
    {

      int haveAnd = args[i][0] == '&' && args[i + 1][0] == '&';
      int haveOr = args[i][0] == '|' && args[i + 1][0] == '|';
      int haveSemicolon = args[i][0] == ';';

      if (haveAnd || haveOr)
      {
        marks++;
        i++;
      }
      else if (haveSemicolon)
      {
        marks++;
      }
      else if (args[i][0] == '|')
      {
        pipes++;
      }
    }

    // No input, continue
    if (args[0] == NULL)
      continue;

    // Check for internal shell commands, such as exit
    if (internal_command(args))
      continue;

    if (pipes == 0 && marks == 0)
    {
      // printf("***********normal************\n");
      exitCode = do_command(args);
    }
    else if (pipes > 0)
    {
      // printf("***********pipe************\n");
      do_command_pipe(args, pipes);
    }
    else if (marks > 0)
    {
      // printf("***********multi************\n");
      do_multi_command(args, marks);
    }
  }
}

/*
 * Check for ampersand as the last argument
 */
int ampersand(char **args)
{
  int i;

  for (i = 1; args[i] != NULL; i++)
    ;

  if (args[i - 1][0] == '&')
  {
    free(args[i - 1]);
    args[i - 1] = NULL;
    return 1;
  }
  else
  {
    return 0;
  }

  return 0;
}

/* 
 * Check for internal commands
 * Returns true if there is more to do, false otherwise 
 */
int internal_command(char **args)
{
  if (strcmp(args[0], "exit") == 0)
  {
    exit(0);
  }

  return 0;
}

/* 
 * Do the command
 */
int do_command(char **args)
{
  int result;
  int block;
  int output;
  int input;
  char *output_filename;
  char *input_filename;

  // Check for an ampersand &
  block = (ampersand(args) == 0);

  // Check for redirected input
  input = redirect_input(args, &input_filename);

  switch (input)
  {
  case -1:
    printf("Syntax error!\n");
    return;
    break;
  case 0:
    break;
  case 1:
    printf("Redirecting input from: %s\n", input_filename);
    break;
  }

  // Check for redirected output
  output = redirect_output(args, &output_filename);

  switch (output)
  {
  case -1:
    printf("Syntax error!\n");
    return;
    break;
  case 0:
    break;
  case 1:
    printf("Redirecting output to: %s\n", output_filename);
    break;
  }

  pid_t child_id;
  int status;

  // Fork the child process
  child_id = fork();

  // Check for errors in fork()
  switch (child_id)
  {
  case EAGAIN:
    perror("Error EAGAIN: ");
    return;
  case ENOMEM:
    perror("Error ENOMEM: ");
    return;
  }

  if (child_id == 0)
  {

    // Set up redirection in the child process
    if (input)
      freopen(input_filename, "r", stdin);

    if (output == 1)
      freopen(output_filename, "w+", stdout);
    else if (output == 2)
    {
      freopen(output_filename, "a+", stdout);
    }

    // Execute the command
    result = execvp(args[0], args);
    exit(-1);
  }
  else if (child_id > 0)
  {
    //bg process, still small mistake, can not stop bg process output to the screen
    struct sigaction s;
    memset(&s, 0, sizeof(s));
    s.sa_handler = sig_handler;
    sigfillset(&s.sa_mask);
    s.sa_flags |= SA_RESTART;
    assert(sigaction(SIGCHLD, &s, NULL) != 1);
  }

  // Wait for the child process to complete, if necessary
  if (block)
  {
    printf("Waiting for child, pid = %d\n", child_id);
    result = waitpid(child_id, &status, 0);
    if (WIFEXITED(status))
    {
      int es = WEXITSTATUS(status);
      if (es == 0)
      {
        // no error
        return 0;
      }
      else
      {
        //error
        return -1;
      }
    }
  }
  else
  {
    sleep(3);
  }
}

/*
 * Check for input redirection
 */
int redirect_input(char **args, char **input_filename)
{
  int i;
  int j;

  for (i = 0; args[i] != NULL; i++)
  {

    // Look for the <
    if (args[i][0] == '<')
    {
      free(args[i]);

      // Read the filename
      if (args[i + 1] != NULL)
      {
        *input_filename = args[i + 1];
      }
      else
      {
        return -1;
      }

      // Adjust the rest of the arguments in the array
      for (j = i; args[j - 1] != NULL; j++)
      {
        args[j] = args[j + 2];
      }

      return 1;
    }
  }

  return 0;
}

/*
 * Check for output redirection
 * return 1 means output
 * return 2 means output append
 */
int redirect_output(char **args, char **output_filename)
{
  int i;
  int j;

  for (i = 0; args[i] != NULL; i++)
  {

    // Look for the >
    if (args[i][0] == '>' && args[i + 1][0] != '>')
    {
      free(args[i]);

      // Get the filename
      if (args[i + 1] != NULL)
      {
        *output_filename = args[i + 1];
      }
      else
      {
        return -1;
      }

      // Adjust the rest of the arguments in the array
      for (j = i; args[j - 1] != NULL; j++)
      {
        args[j] = args[j + 2];
      }

      return 1;
    }
    else if (args[i][0] == '>' && args[i + 1][0] == '>')
    {
      free(args[i]);
      free(args[i + 1]);

      // Get the filename
      if (args[i + 2] != NULL)
      {
        *output_filename = args[i + 2];
      }
      else
      {
        return -1;
      }

      // Adjust the rest of the arguments in the array
      for (j = i; args[j - 1] != NULL; j++)
      {
        args[j] = args[j + 3];
      }

      return 2;
    }
  }

  return 0;
}

int do_command_pipe(char **args, int pipes)
{
  const int commandsNumber = pipes + 1;
  int pipeNumber[2 * pipes];

  int i = 0;
  for (i = 0; i < pipes; i++)
  {
    if (pipe(pipeNumber + i * 2) < 0)
    {
      printf("Couldn't Pipe");
      exit(-1);
    }
  }

  pid_t child_id;
  int status;

  int j = 0;
  int s = 0;
  int index;
  int commandStartIndex[10];

  commandStartIndex[s] = 0;
  s++;
  while (args[i] != NULL)
  {
    if (args[i][0] == '|')
    {
      args[i] = NULL;
      commandStartIndex[s] = i + 1;
      s++;
    }
    i++;
  }

  for (i = 0; i < commandsNumber; ++i)
  {
    index = commandStartIndex[i];

    int result;
    int block;
    int output;
    int input;
    char *output_filename;
    char *input_filename;

    // Check for an ampersand &
    block = (ampersand(args + index) == 0);

    // Check for redirected input
    input = redirect_input(args + index, &input_filename);

    switch (input)
    {
    case -1:
      printf("Syntax error!\n");
      continue;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting input from: %s\n", input_filename);
      break;
    }

    // Check for redirected output
    output = redirect_output(args + index, &output_filename);

    switch (output)
    {
    case -1:
      printf("Syntax error!\n");
      continue;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting output to: %s\n", output_filename);
      break;
    }
    child_id = fork();

    // Check for errors in fork()
    switch (child_id)
    {
    case EAGAIN:
      perror("Error EAGAIN: ");
      return;
    case ENOMEM:
      perror("Error ENOMEM: ");
      return;
    }

    if (child_id == 0)
    {
      // Set up redirection in the child process
      if (input)
        freopen(input_filename, "r", stdin);

      if (output == 1)
        freopen(output_filename, "w+", stdout);
      else if (output == 2)
      {
        freopen(output_filename, "a+", stdout);
      }
      //if not last command
      if (i < pipes)
      {
        if (dup2(pipeNumber[j + 1], 1) < 0)
        {
          perror("dup2");
          exit(-1);
        }
      }

      //if not first command && j!= 2*pipes
      if (j != 0)
      {
        if (dup2(pipeNumber[j - 2], 0) < 0)
        {
          perror("dup2");
          exit(-1);
        }
      }

      for (i = 0; i < 2 * pipes; i++)
      {
        close(pipeNumber[i]);
      }

      result = execvp(args[index], args + index);
      if (result < 0)
      {
        perror(*args);
        exit(-1);
      }
    }
    else if (child_id < 0)
    {
      perror("error");
      exit(-1);
    }

    j += 2;
  }

  // close pipes
  for (i = 0; i < 2 * pipes; i++)
  {
    close(pipeNumber[i]);
  }

  for (i = 0; i < pipes + 1; i++)
  {
    wait(&status);
  }
}

int do_multi_command(char **args, int marks)
{
  const int commands = marks + 1;

  int i = 0;
  int j = 0;
  int s = 1;

  int markType;
  int index;
  int commandStarts[10];
  commandStarts[0] = 0;

  while (args[i] != NULL)
  {
    int haveAnd = args[i][0] == '&' && args[i + 1][0] == '&';
    int haveOr = args[i][0] == '|' && args[i + 1][0] == '|';
    int haveS = args[i][0] == ';';
    if (haveAnd)
    {
      markType = 1;
      args[i] = NULL;
      i++;
      args[i] = NULL;
      commandStarts[s] = i + 1;
      s++;
    }
    else if (haveOr)
    {
      markType = 2;
      args[i] = NULL;
      i++;
      args[i] = NULL;
      commandStarts[s] = i + 1;
      s++;
    }
    else if (haveS)
    {
      markType = 3;
      args[i] = NULL;
      commandStarts[s] = i + 1;
      s++;
    }
    i++;
  }
  if (markType == 1)
  {
    int exitCode = 0;
    for (i = 0; i < commands && exitCode == 0; ++i)
    {
      index = commandStarts[i];
      exitCode = do_command(args + index);
    }
  }
  else if (markType == 2)
  {
    int exitCode = -1;
    for (i = 0; i < commands && exitCode == -1; ++i)
    {
      index = commandStarts[i];
      exitCode = do_command(args + index);
    }
  }
  else if (markType == 3)
  {

    for (i = 0; i < commands; ++i)
    {
      index = commandStarts[i];
      do_command(args + index);
    }
  }
}