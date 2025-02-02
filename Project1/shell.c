#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAXLINE 80
#define HISTORY_COUNT 10

char history[HISTORY_COUNT][MAXLINE];
int historyCount = 0;

void displayHistory() {
  printf("History:\n");
  for (int i = 0; i < historyCount; i++) {
    printf("%d. %s\n", i + 1, history[i]);
  }
}

void add_to_history(char *line) {
  if (historyCount < HISTORY_COUNT) {
    strcpy(history[historyCount], line);
    historyCount++;
  } else {
    for (int i = 0; i < HISTORY_COUNT - 1; i++) {
      strcpy(history[i], history[i + 1]);
    }
    strcpy(history[HISTORY_COUNT - 1], line);
  }
}

int main(int argc, char **argv) {
  if (argc == 2 && equal(argv[1], "--interactive")) {
    return interactiveShell();
  } else {
    return runTests();
  }
}

// Interactive shell to process commands
int interactiveShell() {
  bool should_run = true;
  char *line = calloc(1, MAXLINE);
  
  while (should_run) {
    printf("osh> ");
    fflush(stdout);
    int n = fetchline(&line);
    printf("read: %s (length = %d)\n", line, n);
    
    if (n == -1 || equal(line, "exit")) {
      should_run = false;
      continue;
    }
    
    line[strcspn(line, "\n")] = 0; // Remove trailing newline
    if (strlen(line) == 0) {
      continue; // Skip empty lines
    }

    if (equal(line, "!!")) {
      if (historyCount == 0) {
        printf("No commands in history.\n");
        continue;
      }
      strcpy(line, history[historyCount - 1]);
      printf("%s\n", line);
    } else {
      add_to_history(line);
    }

    // Process the line using the processLine function
    processLine(line);

    char *args[MAXLINE / 2 + 1];
    char *input_file = NULL, *output_file = NULL;
    int background = 0;
    int input_redirect = 0, output_redirect = 0;
    int i = 0;
    char *token = strtok(line, " ");

    while(token != NULL) {
      if (equal(token, "&")) {
        background = 1;
      } else if (equal(token, "<")) {
        input_redirect = 1;
        token = strtok(NULL, " ");
        input_file = token;
      } else if (equal(token, ">")) {
        output_redirect = 1;
        token = strtok(NULL, " ");
        output_file = token;
      } else {
        args[i++] = token;
      }
      token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (args[0] == NULL) {
      continue;
    }

    if (equal(args[0], "history")) {
      displayHistory();
      continue;
    }

    if (equal(args[0], "ascii")) {
      printf("      ()        ****************************    ()\n");
      printf("     (**)       *  \"Violently beautiful\"   *   (oo)\n");
      printf("    (***)       * Vinisha Bala Dhayanidhi  *  (oOo)\n");
      printf("     (**)       *      (vdhaya@uw.edu)     *   (oo)\n");
      printf("      ()        ****************************    ()\n");

      continue;
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("Error during fork");
      continue; 
    }

    if (pid == 0) { // Child process
        if (input_redirect && input_file) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("Input redirection failed");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd); 
        }

        if (output_redirect && output_file) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Output redirection failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(args[0], args); // Execute command
        perror("Exec failed");
        exit(1);
    } else { // Parent process
        if (!background) {
            wait(NULL); // Wait for child process to finish
        }
    }
  }

  free(line);
  return 0;
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

// processLine function to print the line being processed
void processLine(char *line) {
  printf("processing line: %s\n", line);
}

// equal function to compare strings
bool equal(char *a, char *b) { return (strcmp(a, b) == 0); }

// fetchline function to read a line from stdin
int fetchline(char **line) {
  size_t len = 0;
  size_t n = getline(line, &len, stdin);
  if (n > 0) {
    (*line)[n - 1] = '\0'; // Remove trailing newline
  }
  return n;
}