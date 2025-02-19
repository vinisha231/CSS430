#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int pipefd[2];
  pid_t pid;

  // Step 1: Create a pipe
  if (pipe(pipefd) < 0) {
    perror("pipe failed");
    exit(EXIT_FAILURE);
  }

  // Step 2: Fork a child process
  pid = fork();

  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {     // Child process
    close(pipefd[0]); // Close the read end of the pipe

    // Step 3: Redirect stdout to the write end of the pipe
    dup2(pipefd[1], STDOUT_FILENO);

    // Step 4: Execute the command "ps -A"
    execlp("ps", "ps", "-A", (char *)NULL);

    perror("execlp failed");
    exit(EXIT_FAILURE);
  } else {            // Parent process
    close(pipefd[1]); // Close the write end of the pipe

    // Step 5: Read from the pipe and print the output
    char buffer[1024];
    int n;
    while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
      buffer[n] = '\0';
      printf("%s", buffer);
    }

    close(pipefd[0]);
    wait(NULL); // Wait for child to finish
  }

  return 0;
}