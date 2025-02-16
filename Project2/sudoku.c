// Sudoku puzzle verifier and solver
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
  int row;
  int column;
  int psize;
  int **grid;
  bool *result;
} parameters;

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

void *check_row(void *param) {
  parameters *data = (parameters *)param;
  int row = data->row;
  int psize = data->psize;
  int **grid = data->grid;
  bool *result = data->result;

  bool seen[psize + 1];
  for (int i = 1; i <= psize; i++)
    seen[i] = false;

  for (int col = 1; col <= psize; col++) {
    int num = grid[row][col];
    if (num < 1 || num > psize || seen[num]) {
      *result = false;
      pthread_exit(0);
    }
    seen[num] = true;
  }
  *result = true;
  pthread_exit(0);
}

void *check_column(void *param) {
  parameters *data = (parameters *)param;
  int col = data->column;
  int psize = data->psize;
  int **grid = data->grid;
  bool *result = data->result;

  bool seen[psize + 1];
  for (int i = 1; i <= psize; i++)
    seen[i] = false;

  for (int row = 1; row <= psize; row++) {
    int num = grid[row][col];
    if (num < 1 || num > psize || seen[num]) {
      *result = false;
      pthread_exit(0);
    }
    seen[num] = true;
  }
  *result = true;
  pthread_exit(0);
}

void *check_subgrid(void *param) {
  parameters *data = (parameters *)param;
  int startRow = data->row;
  int startCol = data->column;
  int psize = data->psize;
  int **grid = data->grid;
  bool *result = data->result;

  int subgridSize = (int)sqrt(psize);
  bool seen[psize + 1];
  for (int i = 1; i <= psize; i++)
    seen[i] = false;

  for (int row = startRow; row < startRow + subgridSize; row++) {
    for (int col = startCol; col < startCol + subgridSize; col++) {
      int num = grid[row][col];
      if (num < 1 || num > psize || seen[num]) {
        *result = false;
        pthread_exit(0);
      }
      seen[num] = true;
    }
  }
  *result = true;
  pthread_exit(0);
}

void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  *valid = true;
  *complete = true;

  pthread_t threads[psize * 3];
  parameters data[psize * 3];
  bool results[psize * 3];

  int threadIndex = 0;

  for (int i = 1; i <= psize; i++) {
    data[threadIndex].row = i;
    data[threadIndex].column = 0;
    data[threadIndex].psize = psize;
    data[threadIndex].grid = grid;
    data[threadIndex].result = &results[threadIndex];
    pthread_create(&threads[threadIndex], NULL, check_row, &data[threadIndex]);
    threadIndex++;

    data[threadIndex].row = 0;
    data[threadIndex].column = i;
    data[threadIndex].psize = psize;
    data[threadIndex].grid = grid;
    data[threadIndex].result = &results[threadIndex];
    pthread_create(&threads[threadIndex], NULL, check_column,
                   &data[threadIndex]);
    threadIndex++;
  }

  int subgridSize = (int)sqrt(psize);
  for (int row = 1; row <= psize; row += subgridSize) {
    for (int col = 1; col <= psize; col += subgridSize) {
      data[threadIndex].row = row;
      data[threadIndex].column = col;
      data[threadIndex].psize = psize;
      data[threadIndex].grid = grid;
      data[threadIndex].result = &results[threadIndex];
      pthread_create(&threads[threadIndex], NULL, check_subgrid,
                     &data[threadIndex]);
      threadIndex++;
    }
  }

  for (int i = 0; i < threadIndex; i++) {
    pthread_join(threads[i], NULL);
    if (!results[i]) {
      *valid = false;
    }
  }

  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      if (grid[row][col] == 0) {
        *complete = false;
        return;
      }
    }
  }
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}