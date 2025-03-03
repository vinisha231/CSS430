/**
Vinisha Bala Dhayanidhi
CSS 430
Project 1 
2nd February 2025
 */
#include "finalshell.h"
char lastCommand[MAXLINE] = ""; // last command executed
int tokenizeCommand(char *cmd, char *args[], int maxArgs); // tokenize command
void runSingleCommand(char *args[], int argCount, bool background); // run single command
void runBlock(char *block); // run block of commands

int main(int argc, char **argv) {
  if (argc == 2 && equal(argv[1], "--interactive")) { // interactive mode
    return interactiveShell();
  } else {
    return runTests();
  }
}

// interactive shell to process commands
int interactiveShell() {
  bool should_run = true;
  char *line = calloc(1, MAXLINE);
  while (should_run) {
    printf(PROMPT);
    fflush(stdout);
    int n = fetchline(&line);
    printf("read: %s (length = %d)\n", line, n);  
    // ^D results in n == -1
    if (n == -1 || equal(line, "exit")) {
      should_run = false; 
      continue;
    }
    if (equal(line, "")) {
      continue;
    }
    processLine(line);
  }
  free(line);
  return 0;
}

int tokenizeCommand(char *cmd, char *args[], int maxArgs) {
  int argCount = 0; // number of arguments
  char *p = cmd; // pointer to command string
  
  while (*p != NULL) {
    while (isspace((unsigned char)*p)) { // skip leading whitespace
      p++; // move to next character
    }
    // Special Token
    if (*p == '&' || *p == '|' || *p == '<' || *p == '>') {
      args[argCount] = (char *)malloc(2); // allocate memory for special token
      args[argCount][0] = *p; // store special token
      args[argCount][1] = NULL; // null-terminate
      argCount++; // increment argument count
      p++; // move to next character
    } else {
      // Records all until whitespace
      char *start = p;
      while (*p != NULL && !isspace((unsigned char)*p) && *p != '&' && *p != '|' && *p != '<' && *p != '>') {
        p++;
      }
      int len = p - start; // length of token
      args[argCount] = (char *)malloc(len + 1); // allocate memory for token
      strncpy(args[argCount], start, len); // copy token to args
      args[argCount][len] = NULL; // null-terminate
      argCount++;
    }
  }

  args[argCount] = NULL;
  return argCount;
}

/* 
 * runBlock()
 * Splits the tokens of 'block' by background symbol '&'.
 * For each chunk, runSingleCommand(..., backgroundFlag).
 */
void runBlock(char *block) {
  char *args[MAXLINE];
  int argCount = tokenizeCommand(block, args, MAXLINE);
  if (argCount == 0) {
    return;
  }

  if (strcmp(args[argCount - 1], "") == 0) { // remove empty string
    args[argCount - 1] = NULL; // null-terminate
    argCount--;  
  }

  // for (int i = 0; args[i] != NULL; i++) {
  //   printf("args[%d]: %s\n", i, args[i]);
  // }

  int i = 0;
  while (i < argCount) {
    bool background = false; // background flag
    int start = i; // start index
    int end = start; // end index

    while (end < argCount && strcmp(args[end], "&") != 0) { // find end index
      end++;
    }

    if (end < argCount && strcmp(args[end], "&") == 0) { // background command
      background = true; 
    }
    int subCount = end - start;
    if (subCount > 0) {
      char *subArgs[subCount + 1];
      for (int j = 0; j < subCount; j++) { // copy subcommand
        subArgs[j] = args[start + j];
      }
      subArgs[subCount] = NULL; 

      runSingleCommand(subArgs, subCount, background); // run subcommand
    }

    i = end;
    if (i < argCount && strcmp(args[i], "&") == 0) { // skip background symbol
      i++;
    }
  }

  for (int k = 0; k < argCount; k++) {
    free(args[k]);
  }
}

/*
 * runSingleCommand()
 * Checks for pipe '|', input '<', output '>', then forks/exec.
 * If background==false, the parent waits.
 */
void runSingleCommand(char *args[], int argCount, bool background) {
  if (argCount == 0) {
    return;
  }

  if (strcmp(args[0], "ascii") == 0) {
    printf( // some ascii art ;)
        " Violently beautiful\n"
        " Vinisha Bala Dhayanidhi\n"
        "        @@@        \n"
        "     @@@@@@@@@@@    \n"
        "  @@@@@@@@@@@@@@@@@ \n"
        " @@@@@@@@@@@@@@@@@@@\n"
        "  @@@@@@@@@@@@@@@@@ \n"
        "     @@@@@@@@@@@    \n"
        "        @@@        \n"
        "        |||        \n"
        "        |||        \n");
    return;
  }

  int pipePositions[MAXLINE];
  int pipeCount = 0;
  for (int i = 0; i < argCount; i++) {
    if (strcmp(args[i], "|") == 0) { // find pipe positions
      pipePositions[pipeCount++] = i; // store pipe position
    }
  }
  
  if (pipeCount == 0) {
    // if < >
    int inIndex = -1, outIndex = -1;
    for (int i = 0; i < argCount; i++) { // find input/output positions
      if (strcmp(args[i], "<") == 0) { // input
        inIndex = i;
      } else if (strcmp(args[i], ">") == 0) { // output
        outIndex = i;
      }
    }  

    char *inFile = NULL;
    char *outFile = NULL;

    if (inIndex != -1) {
      if (inIndex + 1 < argCount) { // input file
        inFile = args[inIndex + 1];
      }

      args[inIndex] = NULL;
      argCount = inIndex; // update argCount
    }

    if (outIndex != -1) {
      if (outIndex + 1 < argCount) { // output file
        outFile = args[outIndex + 1];
      }

      args[outIndex] = NULL;
      argCount = outIndex; // update argCount
    }

    int pid = fork();
    if (pid < 0) { // fork failed
      perror("fork failed");
      return;
    }

    if (pid == 0) {
      // Child
      // if input file
      if (inFile) {
        FILE *f = fopen(inFile, "r"); // open input file
        if (!f) {
          perror("fopen for input failed");
          exit(EXIT_FAILURE);
        }
        dup2(fileno(f), STDIN_FILENO);
        fclose(f);
      }
      // if output file
      if (outFile) { // open output file
        FILE *f = fopen(outFile, "w");
        if (!f) {
          perror("fopen for output failed");
          exit(EXIT_FAILURE);
        }
        dup2(fileno(f), STDOUT_FILENO);
        fclose(f);
      }

      execvp(args[0], args);
      perror("execvp failed");
      exit(EXIT_FAILURE);
    } else {
      // Parent
      if (!background) {
        waitpid(pid, NULL, 0);
      }
    }
    return; 
  }

  // if pipe
  int numCommands = pipeCount + 1;
  int start = 0;
  char **commands[MAXLINE];
  int cmdArgCounts[MAXLINE];

  for (int c = 0; c < numCommands; c++) {
    int end = (c < pipeCount) ? pipePositions[c] : argCount; // find end index
    int length = end - start;
    commands[c] = (char **)malloc(sizeof(char *) * (length + 1)); // allocate memory
    for (int j = 0; j < length; j++) { // copy command
      commands[c][j] = args[start + j]; 
    }
    commands[c][length] = NULL;
    cmdArgCounts[c] = length;

    start = end + 1;
  }

  int fdPipes[MAXLINE][2];
  for (int i = 0; i < pipeCount; i++) {
    if (pipe(fdPipes[i]) == -1) { // create pipe
      perror("pipe failed");
      return;
    }
  }

  for (int i = 0; i < numCommands; i++) {
    pid_t pid = fork();
    if (pid < 0) { // fork failed
      perror("fork failed");
      return;
    }
    if (pid == 0) {
      // Child i
      // If not the first command, read from the previous pipe
      if (i > 0) {
        dup2(fdPipes[i - 1][0], STDIN_FILENO);
      }

      // If not the last command, write to this pipe
      if (i < pipeCount) {
        dup2(fdPipes[i][1], STDOUT_FILENO);
      }

      // Close all pipes in child
      for (int k = 0; k < pipeCount; k++) {
        close(fdPipes[k][0]);
        close(fdPipes[k][1]);
      }

      // Execute this subcommand
      execvp(commands[i][0], commands[i]);
      perror("execvp (multiple pipes) failed");
      exit(EXIT_FAILURE);
    }
    // Parent continues in loop to spawn next command
  }

  // Parent closes all pipe
  for (int i = 0; i < pipeCount; i++) {
    close(fdPipes[i][0]);
    close(fdPipes[i][1]);
  }

  if (!background) {
    for (int i = 0; i < numCommands; i++) {
      wait(NULL);
    }
  }

  for (int c = 0; c < numCommands; c++) {
    free(commands[c]);
  }  
}

/**
 * processLine()
 * -------------
 * 1) If user typed "!!", run the last command from history (if any).
 * 2) Otherwise, store this line in 'lastCommand' and parse it by semicolons.
 */
void processLine(char *line) {  

  if (strcmp(line, "!!") == 0) { // run last command
    if (lastCommand[0] == NULL) {
      printf("No commands in history.\n");
      return;
    }

    printf("%s\n", lastCommand);
    processLine(lastCommand);
    return;
  }

  strncpy(lastCommand, line, MAXLINE - 1); // store last command
  lastCommand[MAXLINE - 1] = NULL;
  
  char *ptr;
  char *subCmd = strtok_r(line,";", &ptr);


  while (subCmd != NULL) { // run each command in block
    runBlock(subCmd);
    subCmd = strtok_r(NULL, ";", &ptr);
  }

}

int runTests() {
  printf("*** Running basic tests ***\n");
  char lines[7][MAXLINE] = {
      "ls",      "ls -al", "ls & whoami ;", "ls > junk.txt", "cat < junk.txt",
      "ls | wc", "ascii"};
  for (int i = 0; i < 7; i++) {
    printf("* %d. Testing %s *\n", i + 1, lines[i]);
    processLine(lines[i]);
  }

  return 0;
}

// return true if C-strings are equal
bool equal(char *a, char *b) { return (strcmp(a, b) == 0); }

// read a line from console
// return length of line read or -1 if failed to read
// removes the \n on the line read
int fetchline(char **line) {
  size_t len = 0;
  size_t n = getline(line, &len, stdin);
  if (n > 0) {
    (*line)[n - 1] = '\0';
  }
  return n;
}
