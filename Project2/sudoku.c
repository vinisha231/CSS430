/**
Vinisha Bala Dhayanidhi
Sudoku puzzle verifier and solver
Date: 02/16/2025
*/
// compile: gcc -Wall -Wextra sudoku.c -o sudoku -lm -lpthread
// run: ./sudoku puzzle9-valid.txt
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

bool is_valid(int psize, int **grid, int row, int col, int num) {
  for (int x = 1; x <= psize; x++) {
    if (grid[row][x] == num || grid[x][col] == num) {
      return false;
    }
  }

  int subgridSize = (int)sqrt(psize);
  int startRow = row - (row - 1) % subgridSize;
  int startCol = col - (col - 1) % subgridSize;

  for (int r = startRow; r < startRow + subgridSize; r++) {
    for (int c = startCol; c < startCol + subgridSize; c++) {
      if (grid[r][c] == num) {
        return false;
      }
    }
  }
  return true;
}

bool solveSudoku(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      if (grid[row][col] == 0) {
        for (int num = 1; num <= psize; num++) {
          if (is_valid(psize, grid, row, col, num)) {
            grid[row][col] = num;
            if (solveSudoku(psize, grid)) {
              return true;
            }
            grid[row][col] = 0;
          }
        }
        return false;
      }
    }
  }
  return true;
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

  // Check if the puzzle is complete
  checkPuzzle(sudokuSize, grid, &complete, &valid);

  // If the puzzle is not complete, attempt to solve it
  if (!complete) {
    if (solveSudoku(sudokuSize, grid)) {
      printf("Puzzle solved.\n");
    } else {
      printf("Could not solve the puzzle.\n");
    }
    // Re-check the puzzle for completeness and validity after solving
    checkPuzzle(sudokuSize, grid, &complete, &valid);
  }

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