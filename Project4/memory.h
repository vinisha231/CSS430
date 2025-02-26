#ifndef MEMORY_H
#define MEMORY_H

#define MEMSIZE 80

typedef struct Block {
  char process;
  int size;
  struct Block *next;
} Block;

typedef struct {
  Block *head;
} MemoryPool;

void initializeMemoryPool(MemoryPool *pool);
void allocateMemory(MemoryPool *pool, char process, int size, char algo);
void freeMemory(MemoryPool *pool, char process);
void showMemoryPool(MemoryPool *pool);
void readScript(MemoryPool *pool, const char *filename);
void compactMemory(MemoryPool *pool);
void exitProgram(MemoryPool *pool);

#endif // MEMORY_H