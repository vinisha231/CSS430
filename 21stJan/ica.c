#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>
int main() {
  enum { READ, WRITE };
  int pipefd[2];
  int pipeStatus = pipe(pipefd);
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  int pid = fork();
  if (pid == 0) {
    // child
    close(pipefd[READ]);
    dup2(pipefd[WRITE], STDOUT_FILENO);
    printf("Hello World/n");
    // execlp("ps", "ps", "-A", NULL);
    // assert(1 == 2 && "execlp failed");
    // exit(EXIT_FAILURE);
    close(pipefd[WRITE]);
    sleep(10);
    exit(EXIT_SUCCESS);
    // when we fail we exit with child status 256
  } else {
    // parent
    close(pipefd[WRITE]);
    dup2(pipefd[READ], STDIN_FILENO);
    char buffer[8];
    while (true) {
      int n = read(STDIN_FILENO, buffer, sizeof(buffer));
      if (n == 0)
        break;
      for (int i = 0; i < n; i++) {
        if (buffer[i] == ' ') {
          buffer[i] = '_';
        }
        printf("%c", buffer[i]);
      }
    }
    int status;
    wait(&status);
    printf("Child finished with status : %d \n", status);
    close(pipefd[0]);
  }
  return 0;
}

// use execvp(args[0], myArgs) instead of execlp myArgs is an array of each
// command
//  child needs to create a pipe if necessary and write to pipe
//  if there is a redirecection then child needs to use open to open the file
//  and write to it open gives file handle use dup2 to redirect the output to
//  the file handle close the file handle
// telnet uwb.edu 80

// GET / HTTP/1.1
// telnet www.uwb.edu 80