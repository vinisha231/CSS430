#include "shell.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

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

// Execute a single command with optional input/output redirection
void execute_command(char **args, int background) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        int fd;
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], ">") == 0) {
                fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
                args[i] = NULL;
            } else if (strcmp(args[i], "<") == 0) {
                fd = open(args[i + 1], O_RDONLY);
                dup2(fd, STDIN_FILENO);
                close(fd);
                args[i] = NULL;
            }
        }
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        if (!background) {
            waitpid(pid, NULL, 0);
        }
    } else {
        perror("fork");
    }
}

// Execute a pipeline of commands
void execute_pipeline(char **commands, int num_commands) {
    int pipefd[2];
    int prev_pipe_read = -1;

    for (int i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) {
            pipe(pipefd);
        }

        pid_t pid = fork();
        if (pid == 0) {
            if (i > 0) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }
            if (i < num_commands - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                close(pipefd[0]);
            }

            char *args[MAXLINE / 2 + 1];
            int argc = 0;
            char *cmd = strtok(commands[i], " ");
            while (cmd != NULL) {
                args[argc++] = cmd;
                cmd = strtok(NULL, " ");
            }
            args[argc] = NULL;

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else {
            if (i > 0) {
                close(prev_pipe_read);
            }
            if (i < num_commands - 1) {
                prev_pipe_read = pipefd[0];
                close(pipefd[1]);
            }
        }
    }

    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

void processLine(char *line) {
  char *commands[MAXLINE / 2 + 1];   // Array to hold individual commands
  int command_count = 0;
  char *token = strtok(line, ";");   // Split commands by semicolon
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
      args[argc - 1] = NULL;  // Remove '&' from argument list
    }

    // Handle pipeline (|) in the command
    if (strchr(commands[i], '|')) {
      int pipefds[2 * (argc - 1)];  // Create pipes, number of pipes = number of commands - 1
      pid_t pid;
      int j = 0;

      char *subcmd = strtok(commands[i], "|");  // Split the pipeline by '|'
      int prev_pipe_fd = -1;  // Keeps track of the previous pipe's read end

      // Loop through all commands in the pipeline
      while (subcmd != NULL) {
        int k = 0;
        char *pipe_args[MAXLINE / 2 + 1];
        
        // Split the command by space into arguments
        char *pipe_cmd = strtok(subcmd, " ");
        while (pipe_cmd != NULL) {
          pipe_args[k++] = pipe_cmd;
          pipe_cmd = strtok(NULL, " ");
        }
        pipe_args[k] = NULL;  // Null-terminate the arguments array

        // Create a pipe for each command in the pipeline except the last one
        if (i < argc - 1) {
          if (pipe(pipefds + j * 2) == -1) {
            perror("pipe");
            exit(1);
          }
        }

        pid = fork();
        if (pid == 0) {  // Child process
          // If not the first command, take input from the previous pipe
          if (prev_pipe_fd != -1) {
            dup2(prev_pipe_fd, STDIN_FILENO);
            close(prev_pipe_fd);
          }

          // If not the last command, output to the pipe
          if (subcmd != NULL && i < argc - 1) {
            dup2(pipefds[j * 2 + 1], STDOUT_FILENO);  // Write to the pipe
            close(pipefds[j * 2 + 1]);
          }

          // Execute the current command in the pipeline
          execvp(pipe_args[0], pipe_args);
          perror("execvp");
          exit(1);
        } else {  // Parent process
          if (prev_pipe_fd != -1) {
            close(prev_pipe_fd);  // Close the previous pipe read end
          }
          if (subcmd != NULL && i < argc - 1) {
            close(pipefds[j * 2 + 1]);  // Close the current pipe write end
          }
          prev_pipe_fd = pipefds[j * 2];  // Save the current pipe read end for the next command
          j++;  // Move to the next pipe
        }

        subcmd = strtok(NULL, "|");  // Get the next command in the pipeline
      }

      // Wait for all child processes in the pipeline to finish
      for (int m = 0; m < j; m++) {
        wait(NULL);
      }

    } else {
      // If there's no pipeline, just execute the command directly
      pid_t pid = fork();
      if (pid == 0) {  // Child process
        execvp(args[0], args);
        perror("execvp");
        exit(1);
      } else {  // Parent process
        if (!background) {
          waitpid(pid, NULL, 0);  // Wait for the child process to finish
        }
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