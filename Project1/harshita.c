#ifndef SHELL_H
#define SHELL_H

#include <assert.h>  // assert
#include <fcntl.h>   // O_RDWR, O_CREAT
#include <stdbool.h> // bool
#include <stdio.h>   // printf, getline
#include <stdlib.h>  // calloc
#include <string.h>  // strcmp
#include <unistd.h>  // execvp
#include <sys/wait.h> // wait

#define MAXLINE 80
#define PROMPT "osh> "

#define RD 0
#define WR 1

bool equal(char *a, char *b);
int fetchline(char **line);
int interactiveShell();
void processLine(char *line);
void parseCommand(char *line, char **args, int *background);
void addToHistory(char *line);
void handleHistory(char *line);
void handleRedirection(char **args);
void handlePipe(char **args);
void executeCommand(char **args, int background);
void ascii_art();
int main();

#endif

#include "shell.h"

char history[MAXLINE];
int history_count = 0;

int main(int argc, char **argv) {
    if (argc == 2 && equal(argv[1], "--interactive")) {
        return interactiveShell();
    } else if (argc == 1) {
        return interactiveShell();
    } else {
        fprintf(stderr, "Usage: %s [--interactive]\n", argv[0]);
        return 1;
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

void processLine(char *line) {
    addToHistory(line);

    char *args[MAXLINE / 2 + 1]; // Command line arguments
    int background = 0;          // Background flag
    parseCommand(line, args, &background);

    if (strcmp(args[0], "!!") == 0) {
        handleHistory(line);
        parseCommand(line, args, &background);
    } else if (strcmp(args[0], "ascii") == 0) {
        ascii_art();
        return;
    }

    handleRedirection(args);
    handlePipe(args);

    executeCommand(args, background);
}

void parseCommand(char *line, char **args, int *background) {
    int i = 0;
    *background = 0;

    char *token = strtok(line, " ");
    while (token != NULL) {
        if (strcmp(token, "&") == 0) {
            *background = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

void addToHistory(char *line) {
    strcpy(history, line);
    history_count = 1;
}

void handleHistory(char *line) {
    if (history_count == 0) {
        printf("No commands in history.\n");
    } else {
        strcpy(line, history);
        printf("%s\n", line);
    }
}

void handleRedirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            if (fd < 0) {
                perror("Failed to open file");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("Failed to open file");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

void handlePipe(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            char **args1 = args;
            char **args2 = &args[i + 1];

            int fd[2];
            pipe(fd);

            pid_t pid1 = fork();
            if (pid1 < 0) {
                perror("Fork failed");
                exit(1);
            } else if (pid1 == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(args1[0], args1);
                perror("Execution failed");
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 < 0) {
                perror("Fork failed");
                exit(1);
            } else if (pid2 == 0) {
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(args2[0], args2);
                perror("Execution failed");
                exit(1);
            }

            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            wait(NULL);
            return;
        }
        i++;
    }
}

void executeCommand(char **args, int background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("Execution failed");
        }
        exit(1);
    } else {
        // Parent process
        if (!background) {
            wait(NULL);
        }
    }
}

void ascii_art() {
    printf("    (^-^)    \n");
    printf("    (^-~)    \n");
}

// return true if C-strings are equal
bool equal(char *a, char *b) {
    return (strcmp(a, b) == 0);
}

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