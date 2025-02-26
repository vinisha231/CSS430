#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initializeMemoryPool(MemoryPool *pool) { pool->head = NULL; }

void allocateMemory(MemoryPool *pool, char process, int size, char algo) {
  // Implement allocation logic based on the algorithm (First-Fit, Best-Fit,
  // Worst-Fit)
}

void freeMemory(MemoryPool *pool, char process) {
  // Implement logic to free all allocations for the given process
}

void showMemoryPool(MemoryPool *pool) {
  char memory[MEMSIZE + 1];
  memset(memory, '.', MEMSIZE);
  memory[MEMSIZE] = '\0';

  Block *current = pool->head;
  int index = 0;
  while (current != NULL) {
    for (int i = 0; i < current->size; i++) {
      memory[index++] = current->process;
    }
    current = current->next;
  }

  printf("%s\n", memory);
}

void readScript(MemoryPool *pool, const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Error opening file");
    return;
  }

  char command[10];
  char process;
  int size;
  char algo;
  while (fscanf(file, "%s", command) != EOF) {
    if (strcmp(command, "A") == 0) {
      fscanf(file, " %c %d %c", &process, &size, &algo);
      allocateMemory(pool, process, size, algo);
    } else if (strcmp(command, "F") == 0) {
      fscanf(file, " %c", &process);
      freeMemory(pool, process);
    } else if (strcmp(command, "S") == 0) {
      showMemoryPool(pool);
    } else if (strcmp(command, "C") == 0) {
      compactMemory(pool);
    } else if (strcmp(command, "E") == 0) {
      exitProgram(pool);
      break;
    }
  }

  fclose(file);
}

void compactMemory(MemoryPool *pool) {
  // Implement logic to compact the memory pool
}

void exitProgram(MemoryPool *pool) {
  // Implement logic to clean up and exit the program
}