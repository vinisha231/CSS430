#include "shell.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define HISTORY_SIZE 10
#define MAXLINE 80
#define PROMPT "osh> "

char *history[HISTORY_SIZE];
int history_count = 0;

void add_to_history(char *line) {
  if (history_count < HISTORY_SIZE) {
    history[history_count++] = strdup(line);
  } else {
    free(history[0]);
    for (int i = 1; i < HISTORY_SIZE; i++) {
      history[i - 1] = history[i];
    }
    history[HISTORY_SIZE - 1] = strdup(line);
  }
}

void print_history() {
  for (int i = 0; i < history_count; i++) {
    printf("%d %s\n", i + 1, history[i]);
  }
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--interactive") == 0) {
    return interactiveShell();
  } else {
    return runTests();
  }
}

// Function to display ASCII art for "ascii"
void display_ascii_art() {
    printf("\n");
    printf("        @@@        \n");
    printf("     @@@@@@@@@@@    \n");
    printf("  @@@@@@@@@@@@@@@@@ \n");
    printf(" @@@@@@@@@@@@@@@@@@@\n");
    printf("  @@@@@@@@@@@@@@@@@ \n");
    printf("     @@@@@@@@@@@    \n");
    printf("        @@@        \n");
    printf("        |||        \n");
    printf("        |||        \n");
    printf("\n");
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
    if (n == -1 || strcmp(line, "exit") == 0) {
      should_run = false;
      continue;
    }
    if (strcmp(line, "") == 0) {
      continue;
    }
    if (strcmp(line, "!!") == 0) {
      if (history_count == 0) {
        printf("No commands in history.\n");
        continue;
      } else {
        strcpy(line, history[history_count - 1]);
        printf("%s\n", line);
      }
    } else if (strcmp(line, "ascii") == 0) {
      display_ascii_art(); // Display ASCII art when "ascii" is entered
      continue;
    } else {
      add_to_history(line);
    }
    processLine(line);
  }
  free(line);
  return 0;
}

void processLine(char *line) {
  char *commands[MAXLINE / 2 + 1];
  int command_count = 0;
  char *token = strtok(line, ";");
  while (token != NULL) {
    commands[command_count++] = token;
    token = strtok(NULL, ";");
  }

  for (int i = 0; i < command_count; i++) {
    char *args[MAXLINE / 2 + 1];
    int argc = 0;
    char *cmd = strtok(commands[i], " ");
    while (cmd != NULL) {
      args[argc++] = cmd;
      cmd = strtok(NULL, " ");
    }
    args[argc] = NULL;

    int background = 0;
    if (argc > 0 && strcmp(args[argc - 1], "&") == 0) {
      background = 1;
      args[argc - 1] = NULL;
    }

    pid_t pid = fork();
    if (pid == 0) {
      // Child process
      int fd;
      for (int j = 0; j < argc; j++) {
        if (strcmp(args[j], ">") == 0) {
          fd = open(args[j + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
          dup2(fd, STDOUT_FILENO);
          close(fd);
          args[j] = NULL;
          break;
        } else if (strcmp(args[j], "<") == 0) {
          fd = open(args[j + 1], O_RDONLY);
          dup2(fd, STDIN_FILENO);
          close(fd);
          args[j] = NULL;
          break;
        } else if (strcmp(args[j], "|") == 0) {
          int pipefd[2];
          pipe(pipefd);
          pid_t pid2 = fork();
          if (pid2 == 0) {
            // Child process 2
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            args[j] = NULL;
            execvp(args[0], args);
            perror("execvp");
            exit(1);
          } else {
            // Child process 1
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(args[j + 1], &args[j + 1]);
            perror("execvp");
            exit(1);
          }
        }
      }
      execvp(args[0], args);
      perror("execvp");
      exit(1);
    } else {
      // Parent process
      if (!background) {
        waitpid(pid, NULL, 0);
      }
    }
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
