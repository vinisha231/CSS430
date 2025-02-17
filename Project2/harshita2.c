/*
Harshita Raghavan Vithyaa
Project 2
Sudoku Project
*/

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Defining a struct to hold parameters for the thread function
typedef struct {
  int row;      // Row index to be checked
  int col;      // Column index
  int size;     // Size of the Sudoku grid
  int **grid;   // Pointer to a 2D array of the Sudoku grid
  bool *result; // Pointer to a boolean to store if it is valid or not
} parameters;

// Function to check if a given row in the Sudoku grid contains
// unique numbers from 1 to whatever size the user suggests
void *checkRow(void *param) {
  // Casting the input parameter to the struct type
  parameters *params = (parameters *)param;
  int size = params->size;
  int row = params->row;
  bool numbers[size + 1];
  for (int i = 0; i <= size; i++)
    numbers[i] = false;

  for (int col = 0; col < size; col++) {
    int num = params->grid[row][col];
    if (num < 1 || num > size || numbers[num]) {
      // Initializing the array to false - no numbers have been encountered yet
      *(params->result) = false;
      pthread_exit(0);
    }
    numbers[num] = true;
  }
  *(params->result) = true;
  pthread_exit(0);
}

void *checkCol(void *param) {
  parameters *params = (parameters *)param;
  int size = params->size;
  int col = params->col;
  bool numbers[size + 1];
  for (int i = 0; i <= size; i++)
    numbers[i] = false;

  for (int row = 0; row < size; row++) {
    int num = params->grid[row][col];
    if (num < 1 || num > size || numbers[num]) {
      *(params->result) = false;
      pthread_exit(0);
    }
    numbers[num] = true;
  }
  *(params->result) = true;
  pthread_exit(0);
}

// Method to check if a given subgrid in the Sudoku grid contains unique numbers
// from 1 to user size
void *checkSubgrid(void *param) {
  parameters *params = (parameters *)param;
  int size = params->size;
  int row = params->row;
  int col = params->col;
  int subgridSize = (int)sqrt(size);
  // Array to track if numbers 1 to size have been seen
  bool numbers[size + 1];
  for (int i = 0; i <= size; i++)
    numbers[i] = false;

  for (int i = 0; i < subgridSize; i++) {
    for (int j = 0; j < subgridSize; j++) {
      int num = params->grid[row + i][col + j];
      if (num < 1 || num > size || numbers[num]) {
        *(params->result) = false;
        pthread_exit(0);
      }
      numbers[num] = true;
    }
  }
  // If the subgrid is valid, store the success result
  *(params->result) = true;
  pthread_exit(0);
}

// Function to check if the puzzle is complete
bool isComplete(int puzzlesize, int **grid) {
  for (int row = 0; row < puzzlesize; row++) {
    for (int col = 0; col < puzzlesize; col++) {
      if (grid[row][col] == 0)
        return false;
    }
  }
  return true;
}

// Method to fill a single missing number
bool singleMissingNumber(int puzzleSize, int **grid) {
  bool filled = false;
  for (int row = 0; row < puzzleSize && !filled; row++) {
    for (int col = 0; col < puzzleSize && !filled; col++) {
      if (grid[row][col] == 0) {
        // Boolean array to track which numbers are already present in the
        // row, column, and subgrid
        bool digits[puzzleSize + 1];
        for (int i = 0; i <= puzzleSize; i++)
          digits[i] = false;
        for (int i = 0; i < puzzleSize; i++) {
          // Mark as already present in the row and column
          if (grid[row][i] != 0)
            digits[grid[row][i]] = true;
          if (grid[i][col] != 0)
            digits[grid[i][col]] = true;
        }

        int subgridSize = (int)sqrt(puzzleSize);
        int startRow = (row / subgridSize) * subgridSize;
        int startCol = (col / subgridSize) * subgridSize;
        for (int i = 0; i < subgridSize; i++) {
          for (int j = 0; j < subgridSize; j++) {
            if (grid[startRow + i][startCol + j] != 0)
              digits[grid[startRow + i][startCol + j]] = true;
          }
        }
        // Counting how many numbers are possible for this cell
        int count = 0, num = 0;
        for (int i = 1; i <= puzzleSize; i++) {
          if (!digits[i]) {
            count++;
            num = i;
          }
        }
        // If only 1 possible number exists, fill in the cell with values
        if (count == 1) {
          grid[row][col] = num;
          filled = true;
        }
      }
    }
  }
  return filled;
}

// Method to check the validity of a Sudoku puzzle
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  *complete = isComplete(psize, grid);
  *valid = true;

  // If not complete, try to fill in missing numbers
  if (!*complete) {
    while (singleMissingNumber(psize, grid)) {
    }
    *complete = isComplete(psize, grid);
  }

  // If the puzzle is complete, validate the solution
  if (*complete) {
    int numThreads = psize * 2 + (int)sqrt(psize) * (int)sqrt(psize);
    pthread_t threads[numThreads];
    parameters data[numThreads];
    bool results[numThreads];
    int threadIndex = 0;

    // Threads to check rows
    for (int i = 0; i < psize; i++) {
      data[threadIndex] = (parameters){.row = i,
                                       .col = 0,
                                       .size = psize,
                                       .grid = grid,
                                       .result = &results[threadIndex]};
      pthread_create(&threads[threadIndex], NULL, checkRow, &data[threadIndex]);
      threadIndex++;
    }

    // Threads to check columns
    for (int i = 0; i < psize; i++) {
      data[threadIndex] = (parameters){.row = 0,
                                       .col = i,
                                       .size = psize,
                                       .grid = grid,
                                       .result = &results[threadIndex]};
      pthread_create(&threads[threadIndex], NULL, checkCol, &data[threadIndex]);
      threadIndex++;
    }

    // Threads to check subgrids
    int subgridSize = (int)sqrt(psize);
    for (int i = 0; i < subgridSize; i++) {
      for (int j = 0; j < subgridSize; j++) {
        data[threadIndex] = (parameters){.row = i * subgridSize,
                                         .col = j * subgridSize,
                                         .size = psize,
                                         .grid = grid,
                                         .result = &results[threadIndex]};
        pthread_create(&threads[threadIndex], NULL, checkSubgrid,
                       &data[threadIndex]);
        threadIndex++;
      }
    }

    for (int i = 0; i < numThreads; i++) {
      pthread_join(threads[i], NULL);
      if (!results[i]) {
        *valid = false;
        break; // Exit early if invalid
      }
    }
  } else {
    *valid = false;
  }
}

// Function to read Sudoku puzzle from a file
int readSudokuPuzzle(const char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  int psize;
  fscanf(fp, "%d", &psize);

  // Dynamically allocate memory for the grid
  int **agrid = (int **)malloc(psize * sizeof(int *));
  if (!agrid) {
    printf("Memory allocation failed for grid\n");
    exit(EXIT_FAILURE);
  }

  // Allocate memory for each row of the grid
  for (int row = 0; row < psize; row++) {
    agrid[row] = (int *)malloc(psize * sizeof(int));
    if (!agrid[row]) {
      printf("Memory allocation failed for row %d\n", row);
      exit(EXIT_FAILURE);
    }
    // Read each element of the row
    for (int col = 0; col < psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }

  fclose(fp);
  *grid = agrid;
  return psize;
}

// Prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 0; row < psize; row++) {
    for (int col = 0; col < psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// Frees the memory allocated
void deleteSudokuPuzzle(int puzzleSize, int **grid) {
  for (int row = 0; row < puzzleSize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// Main Function - implemented from the starter code from Prof Pisan
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  int **grid = NULL;
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