/**
 * Project 4 memory allocation
 * Vinisha Bala Dhayanidhi
 * 03/02/2025
 */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXCHAR 80          // maximum number of tokens for args and argsr
#define MEMSIZE 80          // size of buffer
#define SIZE 256            // size of command
char buffer[MEMSIZE];       // main buffer
char extra_buffer[MEMSIZE]; // extra buffer for the 'C' function
char *args[MAXCHAR];        // array for tokens
char *argsr[MAXCHAR];       // array for tokens for R command
int should_run = 1;         // contoling while loop variable
// execution function which works with args[] based on command type
void execution() {
  // show function
  if (strcmp(args[0], "S") == 0) {
    for (int i = 0; i < MEMSIZE; i++) {
      printf("%c", buffer[i]);
    }
    printf("\n");
  }
  // allocate function
  if (strcmp(args[0], "A") == 0) {
    int available_spaces = 0; // variable to count available spaces
    // declaring all necessary variables from the user command
    char *command_type = args[0];
    char *process_name = args[1];
    int how_many = atoi(args[2]);
    char *algo_type = args[3];
    // first-fit algo
    if (strcmp(algo_type, "F") == 0) {
      for (int i = 0; i < MEMSIZE; i++) {
        if (buffer[i] == '.') {
          available_spaces++;
          if (available_spaces == how_many) {
            for (int k = i - how_many + 1; k < i + 1; k++) {
              buffer[k] = *process_name;
            }
            available_spaces = 0;
            break;
          }
          // case when not enough memory
          if (i == 79 && available_spaces < how_many) {
            printf("Not enough space for process: %s\n", process_name);
          }
        } else {
          available_spaces = 0;
        }
      }
    }
    // best-fit algo
    if (strcmp(algo_type, "B") == 0) {
      int best_index;
      int best_space = 100;
      for (int i = 0; i < MEMSIZE; i++) {
        if (buffer[i] == '.') {
          available_spaces++;
          // case if buffer ends with '.'
          if (i == 79) {
            if (available_spaces < best_space && available_spaces >= how_many) {
              best_space = available_spaces;
              best_index = i - available_spaces + 1;
            }
            available_spaces = 0;
          }
        }
        // when buffer change from process to '.' we are updating best_space and
        // best_index
        else {
          if (available_spaces < best_space && available_spaces >= how_many) {
            best_space = available_spaces;
            best_index = i - available_spaces;
          }
          available_spaces = 0;
        }
      }
      // case when not enough memory
      if (best_space == 100) {
        printf("Not enough space for process: %s\n", process_name);
      } else {
        // inserting processes in buffer based on best_index
        for (int j = best_index; j < how_many + best_index; j++) {
          buffer[j] = *process_name;
        }
      }
    }
    // worst-fit algo
    if (strcmp(algo_type, "W") == 0) {
      int best_index;
      int best_space = -1;
      for (int i = 0; i < MEMSIZE; i++) {
        if (buffer[i] == '.') {
          available_spaces++;
          // case if buffer ends with '.'
          if (i == 79) {
            if (available_spaces > best_space && available_spaces >= how_many) {
              best_space = available_spaces;
              best_index = i - available_spaces + 1;
            }
            available_spaces = 0;
          }
          // when buffer change from process to '.' we are updating best_space
          // and best_index
        } else {
          if (available_spaces > best_space && available_spaces >= how_many) {
            best_space = available_spaces;
            best_index = i - available_spaces;
          }
          available_spaces = 0;
        }
      }
      // case when not enough memory
      if (best_space == -1) {
        printf("Not enough space for process: %s\n", process_name);
      } else {
        // inserting processes in buffer based on best_index
        for (int j = best_index; j < how_many + best_index; j++) {
          buffer[j] = *process_name;
        }
      }
    }
  }
  // command to free allocated memory
  if (strcmp(args[0], "F") == 0) {
    int mark = 0;
    // declaring variables
    char *command_type = args[0];
    char *process_name = args[1];
    // going through the loop and deleting processes based on process_name
    for (int i = 0; i < MEMSIZE; i++) {
      if (buffer[i] == *process_name) {
        buffer[i] = '.';
        mark = 1;
      }
    }
    if (mark == 0) {
      printf("No processes found!\n");
    }
  }
  // compaction command
  if (strcmp(args[0], "C") == 0) {
    // decalring m variable to keep trak of next index for the extra buffer
    int m = 0;
    // adding all variables in a row
    for (int i = 0; i < MEMSIZE; i++) {
      if (buffer[i] != '.') {
        extra_buffer[m] = buffer[i];
        m++;
      }
    }
    // deleting previos buffer elements adding all processes in the main buffer
    // and deleting all elemnts from extra buffer
    for (int i = 0; i < MEMSIZE; i++) {
      buffer[i] = '.';
      buffer[i] = extra_buffer[i];
      extra_buffer[i] = '.';
    }
  }
  // exit command
  if (strcmp(args[0], "E") == 0) {
    should_run = 0;
  }
}
// function which checks that args[2] is an integer
bool isNumber(char number[]) {
  int i = 0;
  if (number[0] == '-')
    i = 1;
  for (; number[i] != 0; i++) {

    if (!isdigit(number[i]))
      return false;
  }
  return true;
}

int main() {
  // filling out main and extra buffer with the '.'
  for (int i = 0; i < MEMSIZE; i++) {
    buffer[i] = '.';
    extra_buffer[i] = '.';
  }
  // while loop which ask user for the commands
  while (should_run) {
    printf("Your command: ");
    fflush(stdout);
    char cmd[MAXCHAR + 1];
    // getting output from user
    fgets(cmd, MAXCHAR + 1, stdin);
    // exit command
    if (strncmp(cmd, "exit", 4) == 0) {
      should_run = 0;
    }
    // 'R' command
    if (strncmp(cmd, "R", 1) == 0) {
      cmd[strlen(cmd) - 1] = 0;
      // splitting command by two tokens where the second one is name of the
      // file
      char *t = strtok(cmd, " ");
      int i = 0;
      while (t != 0) {
        argsr[i] = t;
        t = strtok(0, " ");
        i++;
      }
      argsr[i] = NULL;
      FILE *fp;
      // opening a file
      fp = fopen(argsr[1], "r");
      char line[SIZE + 1];
      // if file opened
      if (fp) {
        // reading file line by line
        while (fgets(line, SIZE + 1, fp) != NULL && should_run) {
          // if case for not getting a seg fault if last line don't have
          // anything
          if (strlen(line) > 1)
            line[strlen(line) - 1] = '\0';
          else
            line[1] = ' ';
          char *token = strtok(line, " ");
          int k = 0;
          // spliting by tokens each line
          while (token != 0) {
            args[k] = token;
            token = strtok(0, " ");
            k++;
          }
          args[k] = NULL;
          // executing each line
          execution();
        }
        // error if file not opened
      } else {
        printf("Could not open file %s\n", argsr[1]);
        return 1;
      }
      // closing file
      fclose(fp);
    }
    // case for all command except 'R memo.txt'
    if ((strncmp(cmd, "A", 1) == 0) || (strncmp(cmd, "F", 1) == 0) ||
        (strncmp(cmd, "C", 1) == 0) || (strncmp(cmd, "S", 1) == 0) ||
        (strncmp(cmd, "E", 1) == 0)) {
      cmd[strlen(cmd) - 1] = 0;
      char *t = strtok(cmd, " ");
      int i = 0;
      // spliting by tokens
      while (t != 0) {
        args[i] = t;
        t = strtok(0, " ");
        i++;
      }
      args[i] = NULL;
      // if statements for the excecution command which checking that command is
      // valid
      if (((strncmp(args[0], "A", 1) == 0) && isNumber(args[2]) &&
           ((strncmp(args[3], "F", 1) == 0) ||
            (strncmp(args[3], "B", 1) == 0) ||
            (strncmp(args[3], "W", 1) == 0)))) {
        execution();
      } else if ((strncmp(args[0], "S", 1) == 0) && !args[1]) {
        execution();
      } else if ((strncmp(args[0], "C", 1) == 0) && !args[1]) {
        execution();
      } else if ((strncmp(args[0], "F", 1) == 0) && (args[1])) {
        execution();
      } else if ((strncmp(args[0], "E", 1) == 0) && !args[1]) {
        should_run = 0;
      } else {
        printf("Invalid command\n");
      }

    } else if (strncmp(cmd, "exit", 4) == 0) {
      continue;
    } else if (strncmp(cmd, "R", 1) == 0) {
      continue;
    } else if (strncmp(cmd, "E", 1) == 0) {
      continue;
    } else {
      printf("Invalid command\n");
    }
  }
  return 0;
}