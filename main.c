#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
const int BUF_SIZE = 4096;
int main() {
  enum { READ, WRITE };
  pid_t pid;
  int pipeFD[2];
  if (pipe(pipeFD) < 0) {
    perror("Error in creating pipe");
    exit(EXIT_FAILURE);
  }
  pid = fork();
  if (pid < 0) {
    perror("Error during fork");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    close(pipeFD[READ]);
    dup2(pipeFD[WRITE], 1);
    execlp("/bin/ls", "ls", "-l", NULL);
  } else {
    wait(NULL);
    char buf[BUF_SIZE];
    int n = read(pipeFD[READ], buf, BUF_SIZE);
    buf[n] = '\0';
    for (int i = 0; i < n; i++) {
      printf(buf[i] + " ");
      printf("Parent exiting\n");
    }
  }
  exit(EXIT_SUCCESS);
}