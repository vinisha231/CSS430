#include "shell.h"
#include <sys/wait.h> // For wait

#define HISTORY_SIZE 10

char history[HISTORY_SIZE][MAXLINE];
int history_count = 0;

void add_to_history(char *line) {
    if (history_count < HISTORY_SIZE) {
        strcpy(history[history_count], line);
        history_count++;
    } else {
        // Shift history to make room for new command
        for (int i = 1; i < HISTORY_SIZE; i++) {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[HISTORY_SIZE - 1], line);
    }
}

void processLine(char *line) {
    char *args1[MAXLINE / 2 + 1];
    char *args2[MAXLINE / 2 + 1];
    int background = 0;
    int redirect_in = 0;
    int redirect_out = 0;
    int pipe_flag = 0;
    char *input_file = NULL;
    char *output_file = NULL;

    // Add command to history, handle !!
    if (strcmp(line, "!!") == 0) {
        if (history_count == 0) {
            printf("No commands in history\n");
            return;
        } else {
            printf("%s\n", history[history_count - 1]);
            strcpy(line, history[history_count - 1]);
        }
    } else {
        add_to_history(line);
    }

    // Split the input line by semicolon to handle command separation
    char *command = strtok(line, ";");
    while (command != NULL) {
        int i = 0;
        int j = 0;
        background = 0; // Reset background flag for each command
        pipe_flag = 0; // Reset pipe flag for each command
        redirect_in = 0; // Reset redirect in flag for each command
        redirect_out = 0; // Reset redirect out flag for each command
        input_file = NULL; // Reset input file for each command
        output_file = NULL; // Reset output file for each command

        char *token = strtok(command, " ");
        while (token != NULL) {
            if (strcmp(token, "<") == 0) {
                redirect_in = 1;
                token = strtok(NULL, " ");
                input_file = token;
            } else if (strcmp(token, ">") == 0) {
                redirect_out = 1;
                token = strtok(NULL, " ");
                output_file = token;
            } else if (strcmp(token, "|") == 0) {
                pipe_flag = 1;
                args1[i] = NULL;
                i = 0;
            } else if (strcmp(token, "&") == 0) {
                background = 1;
                break; // No need to process further tokens in this command
            } else {
                if (pipe_flag) {
                    args2[i++] = token;
                } else {
                    args1[i++] = token;
                }
            }
            token = strtok(NULL, " ");
        }
        if (pipe_flag) {
            args2[i] = NULL;
        } else {
            args1[i] = NULL;
        }
        
         // Check for "ascii" command
        if (strcmp(line, "ascii") == 0) {
            ascii_art();
            return;
        }
    
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return;
        } else if (pid == 0) {
            // Child process
            if (pipe_flag) {
                int fd[2];
                if (pipe(fd) == -1) {
                    perror("pipe failed");
                    exit(EXIT_FAILURE);
                }
                pid_t pid2 = fork();
                if (pid2 < 0) {
                    perror("fork failed");
                    exit(EXIT_FAILURE);
                } else if (pid2 == 0) {
                    close(fd[1]); // Close write end in second process
                    dup2(fd[0], STDIN_FILENO); // Redirect stdin to pipe
                    close(fd[0]);
                    if (execvp(args2[0], args2) == -1) {
                        perror("execvp failed");
                    }
                    exit(EXIT_FAILURE);
                } else {
                    close(fd[0]); // Close read end in parent
                    dup2(fd[1], STDOUT_FILENO); // Redirect stdout to pipe
                    close(fd[1]);
                    if (execvp(args1[0], args1) == -1) {
                        perror("execvp failed");
                    }
                    exit(EXIT_FAILURE);
                }
            } else {
                // Handle input/output redirection
                if (redirect_in) {
                    int fd = open(input_file, O_RDONLY);
                    if (fd == -1) {
                        perror("open failed");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                if (redirect_out) {
                    int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open failed");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                if (execvp(args1[0], args1) == -1) {
                    perror("execvp failed");
                }
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process
            if (!background) {
                wait(NULL);
            }
        }

        // Move to the next command separated by semicolon
        command = strtok(NULL, ";");
    }
}

int main(int argc, char **argv) {
    return interactiveShell();
}

// interactive shell to process commands
int interactiveShell() {
    bool should_run = true;
    char *line = calloc(1, MAXLINE);
    while (should_run) {
        printf(PROMPT);
        fflush(stdout);
        int n = fetchline(&line);
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

void ascii_art() {
    printf("    (^-^)    \n");
    printf("    (^-~)    \n");
}