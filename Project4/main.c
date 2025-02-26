#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  MemoryPool pool;
  initializeMemoryPool(&pool);

  char command[10];
  char process;
  int size;
  char algo;

  while (1) {
    printf("Enter command: ");
    scanf("%s", command);

    if (strcmp(command, "A") == 0) {
      scanf(" %c %d %c", &process, &size, &algo);
      allocateMemory(&pool, process, size, algo);
    } else if (strcmp(command, "F") == 0) {
      scanf(" %c", &process);
      freeMemory(&pool, process);
    } else if (strcmp(command, "S") == 0) {
      showMemoryPool(&pool);
    } else if (strcmp(command, "R") == 0) {
      char filename[100];
      scanf("%s", filename);
      readScript(&pool, filename);
    } else if (strcmp(command, "C") == 0) {
      compactMemory(&pool);
    } else if (strcmp(command, "E") == 0) {
      exitProgram(&pool);
      break;
    } else {
      printf("Invalid command\n");
    }
  }

  return 0;
}