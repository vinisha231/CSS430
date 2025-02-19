#include <stdio.h>
#include <stdlib.h>   // for exit
#include <sys/wait.h> // for wait
#include <unistd.h>   // for fork, execlp, sleep

int main(int argc, char *argv[]) {
  int status;
  int pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Error: Fork Failed\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    // Child
    printf("Child Sleeping: \n");
    sleep(10);
    int rc = execlp("/bin/ls", "ls", "-l", (char *)0);
    if (rc == -1) {
      perror("Error on execlp");
      exit(EXIT_FAILURE);
    }
    // If exec() succeeds, it never returns
    // the line below would only get executed if exec failed
    printf("Child failed\n");
    exit(EXIT_FAILURE);
  } else {
    // Parent
    printf("Parent Waiting\n");
    if (wait(&status) == -1) {
      perror("Error on wait");
      exit(EXIT_FAILURE);
    }
    printf("Parent Sleeping\n");
    sleep(3);
    exit(EXIT_SUCCESS);
  }
}