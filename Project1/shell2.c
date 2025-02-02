#include "shell.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define HISTORY_SIZE 10

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
  if (argc == 2 && equal(argv[1], "--interactive")) {
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
    if (equal(line, "!!")) {
      if (history_count == 0) {
        printf("No commands in history.\n");
        continue;
      } else {
        strcpy(line, history[history_count - 1]);
        printf("%s\n", line);
      }
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
    int wait_for_child = 1; // By default, wait for the child
    if (argc > 0 && equal(args[argc - 1], "&")) {
      background = 1;
      args[argc - 1] = NULL;
      wait_for_child = 0; // Don't wait if it ends with "&"
    } else if (i == command_count - 1) {
      // If it's the last command and does not end with ";" or "&", treat it as ";"
      if (!equal(commands[i] + strlen(commands[i]) - 1, ";")) {
        wait_for_child = 1;
      }
    }

    pid_t pid = fork();
    if (pid == 0) {
      // Child process
      int fd;
      for (int j = 0; j < argc; j++) {
        if (equal(args[j], ">")) {
          fd = open(args[j + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
          dup2(fd, STDOUT_FILENO);
          close(fd);
          args[j] = NULL;
          break;
        } else if (equal(args[j], "<")) {
          fd = open(args[j + 1], O_RDONLY);
          dup2(fd, STDIN_FILENO);
          close(fd);
          args[j] = NULL;
          break;
        } else if (equal(args[j], "|")) {
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
      if (wait_for_child) {
        waitpid(pid, NULL, 0);
      } else {
        // If the command ends with "&", do not wait
        continue; // Do not wait and continue to the next command
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
// ps auxf | cat | tac | cat | tac | grep `whoami`
