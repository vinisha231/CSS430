// Sudoku puzzle verifier and solver

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int row;
  int col;
  int size;
  int **grid;
  bool *result;
} parameters;

void *checkRow(void *param) {
  parameters *params = (parameters *)param;
  int size = params->size;
  int row = params->row;
  bool digits[size + 1];
  for (int i = 1; i <= size; i++)
    digits[i] = false;

  for (int col = 1; col <= size; col++) {
    int num = params->grid[row][col];
    if (num < 1 || num > size || digits[num]) {
      *(params->result) = false;
      pthread_exit(0);
    }
    digits[num] = true;
  }
  *(params->result) = true;
  pthread_exit(0);
}

void *checkCol(void *param) {
  parameters *params = (parameters *)param;
  int size = params->size;
  int col = params->col;
  bool digits[size + 1];
  for (int i = 1; i <= size; i++)
    digits[i] = false;

  for (int row = 1; row <= size; row++) {
    int num = params->grid[row][col];
    if (num < 1 || num > size || digits[num]) {
      *(params->result) = false;
      pthread_exit(0);
    }
    digits[num] = true;
  }
  *(params->result) = true;
  pthread_exit(0);
}

void *checkSubgrid(void *param) {
  parameters *params = (parameters *)param;
  int size = params->size;
  int row = params->row;
  int col = params->col;
  int subgridSize = (int)sqrt(size);
  bool digits[size + 1];
  for (int i = 1; i <= size; i++)
    digits[i] = false;

  for (int i = 0; i < subgridSize; i++) {
    for (int j = 0; j < subgridSize; j++) {
      int num = params->grid[row + i][col + j];
      if (num < 1 || num > size || digits[num]) {
        *(params->result) = false;
        pthread_exit(0);
      }
      digits[num] = true;
    }
  }
  *(params->result) = true;
  pthread_exit(0);
}

// Function to check if the puzzle is complete
bool isComplete(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      if (grid[row][col] == 0)
        return false;
    }
  }
  return true;
}

// Function to fill a single missing number in an easy puzzle
bool fillSingleMissingNumber(int psize, int **grid) {
  bool filled = false;
  for (int row = 1; row <= psize && !filled; row++) {
    for (int col = 1; col <= psize && !filled; col++) {
      if (grid[row][col] == 0) {
        bool digits[psize + 1];
        for (int i = 1; i <= psize; i++)
          digits[i] = false;

        for (int i = 1; i <= psize; i++) {
          if (grid[row][i] != 0)
            digits[grid[row][i]] = true;
          if (grid[i][col] != 0)
            digits[grid[i][col]] = true;
        }

        int subgridSize = (int)sqrt(psize);
        int startRow = (row - 1) / subgridSize * subgridSize + 1;
        int startCol = (col - 1) / subgridSize * subgridSize + 1;
        for (int i = 0; i < subgridSize; i++) {
          for (int j = 0; j < subgridSize; j++) {
            if (grid[startRow + i][startCol + j] != 0)
              digits[grid[startRow + i][startCol + j]] = true;
          }
        }

        int count = 0, num = 0;
        for (int i = 1; i <= psize; i++) {
          if (!digits[i]) {
            count++;
            num = i;
          }
        }

        if (count == 1) {
          grid[row][col] = num;
          filled = true;
        }
      }
    }
  }
  return filled;
}

void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  *complete = isComplete(psize, grid);
  *valid = true;

  if (!*complete) {
    while (fillSingleMissingNumber(psize, grid)) {
    }
    *complete = isComplete(psize, grid);
  }

  if (*complete) {
    int numThreads =
        psize + psize + (psize / (int)sqrt(psize)) * (psize / (int)sqrt(psize));
    pthread_t threads[numThreads];
    parameters data[numThreads];
    bool results[numThreads];

    int threadIndex = 0;

    for (int i = 1; i <= psize; i++) {
      data[threadIndex].row = i;
      data[threadIndex].col = 0;
      data[threadIndex].size = psize;
      data[threadIndex].grid = grid;
      data[threadIndex].result = &results[threadIndex];
      pthread_create(&threads[threadIndex], NULL, checkRow, &data[threadIndex]);
      threadIndex++;
    }

    for (int i = 1; i <= psize; i++) {
      data[threadIndex].row = 0;
      data[threadIndex].col = i;
      data[threadIndex].size = psize;
      data[threadIndex].grid = grid;
      data[threadIndex].result = &results[threadIndex];
      pthread_create(&threads[threadIndex], NULL, checkCol, &data[threadIndex]);
      threadIndex++;
    }

    int subgridSize = (int)sqrt(psize);
    for (int i = 0; i < subgridSize; i++) {
      for (int j = 0; j < subgridSize; j++) {
        data[threadIndex].row = i * subgridSize + 1;
        data[threadIndex].col = j * subgridSize + 1;
        data[threadIndex].size = psize;
        data[threadIndex].grid = grid;
        data[threadIndex].result = &results[threadIndex];
        pthread_create(&threads[threadIndex], NULL, checkSubgrid,
                       &data[threadIndex]);
        threadIndex++;
      }
    }

    for (int i = 0; i < numThreads; i++) {
      pthread_join(threads[i], NULL);
      if (!results[i]) {
        *valid = false;
      }
    }
  } else {
    *valid = false;
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