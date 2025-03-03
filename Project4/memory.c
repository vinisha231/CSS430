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

#define MAXCHAR 80
#define MEMSIZE 80
#define SIZE 256
char buffer[MEMSIZE];
char extra_buffer[MEMSIZE];
char *args[MAXCHAR];
char *argsr[MAXCHAR];
int should_run = 1;
void execution() {
  if (strcmp(args[0], "S") == 0) {
    for (int i = 0; i < MEMSIZE; i++) {
      printf("%c", buffer[i]);
    }
    printf("\n");
  }
  if (strcmp(args[0], "A") == 0) {
    int available_spaces = 0;
    char *command_type = args[0];
    char *process_name = args[1];
    int how_many = atoi(args[2]);
    char *algo_type = args[3];
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
          if (i == 79 && available_spaces < how_many) {
            printf("Not enough space for process: %s\n", process_name);
          }
        } else {
          available_spaces = 0;
        }
      }
    }
    if (strcmp(algo_type, "B") == 0) {
      int best_index;
      int best_space = 100;
      for (int i = 0; i < MEMSIZE; i++) {
        if (buffer[i] == '.') {
          available_spaces++;
          if (i == 79) {
            if (available_spaces < best_space && available_spaces >= how_many) {
              best_space = available_spaces;
              best_index = i - available_spaces + 1;
            }
            available_spaces = 0;
          }
        } else {
          if (available_spaces < best_space && available_spaces >= how_many) {
            best_space = available_spaces;
            best_index = i - available_spaces;
          }
          available_spaces = 0;
        }
      }
      if (best_space == 100) {
        printf("Not enough space for process: %s\n", process_name);
      } else {
        for (int j = best_index; j < how_many + best_index; j++) {
          buffer[j] = *process_name;
        }
      }
    }
    if (strcmp(algo_type, "W") == 0) {
      int best_index;
      int best_space = -1;
      for (int i = 0; i < MEMSIZE; i++) {
        if (buffer[i] == '.') {
          available_spaces++;
          if (i == 79) {
            if (available_spaces > best_space && available_spaces >= how_many) {
              best_space = available_spaces;
              best_index = i - available_spaces + 1;
            }
            available_spaces = 0;
          }
        } else {
          if (available_spaces > best_space && available_spaces >= how_many) {
            best_space = available_spaces;
            best_index = i - available_spaces;
          }
          available_spaces = 0;
        }
      }
      if (best_space == -1) {
        printf("Not enough space for process: %s\n", process_name);
      } else {
        for (int j = best_index; j < how_many + best_index; j++) {
          buffer[j] = *process_name;
        }
      }
    }
  }
  if (strcmp(args[0], "F") == 0) {
    int mark = 0;
    char *command_type = args[0];
    char *process_name = args[1];
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
  if (strcmp(args[0], "C") == 0) {
    int m = 0;
    for (int i = 0; i < MEMSIZE; i++) {
      if (buffer[i] != '.') {
        extra_buffer[m] = buffer[i];
        m++;
      }
    }
    for (int i = 0; i < MEMSIZE; i++) {
      buffer[i] = '.';
      buffer[i] = extra_buffer[i];
      extra_buffer[i] = '.';
    }
  }
  if (strcmp(args[0], "E") == 0) {
    should_run = 0;
  }
}
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
  for (int i = 0; i < MEMSIZE; i++) {
    buffer[i] = '.';
    extra_buffer[i] = '.';
  }
  while (should_run) {
    printf("Your command: ");
    fflush(stdout);
    char cmd[MAXCHAR + 1];
    fgets(cmd, MAXCHAR + 1, stdin);
    if (strncmp(cmd, "exit", 4) == 0) {
      should_run = 0;
    }
    if (strncmp(cmd, "R", 1) == 0) {
      cmd[strlen(cmd) - 1] = 0;
      char *t = strtok(cmd, " ");
      int i = 0;
      while (t != 0) {
        argsr[i] = t;
        t = strtok(0, " ");
        i++;
      }
      argsr[i] = NULL;
      FILE *fp;
      fp = fopen(argsr[1], "r");
      char line[SIZE + 1];
      if (fp) {
        while (fgets(line, SIZE + 1, fp) != NULL && should_run) {
          if (strlen(line) > 1)
            line[strlen(line) - 1] = '\0';
          else
            line[1] = ' ';
          char *token = strtok(line, " ");
          int k = 0;
          while (token != 0) {
            args[k] = token;
            token = strtok(0, " ");
            k++;
          }
          args[k] = NULL;
          execution();
        }
      } else {
        printf("Could not open file %s\n", argsr[1]);
        return 1;
      }
      fclose(fp);
    }
    if ((strncmp(cmd, "A", 1) == 0) || (strncmp(cmd, "F", 1) == 0) ||
        (strncmp(cmd, "C", 1) == 0) || (strncmp(cmd, "S", 1) == 0) ||
        (strncmp(cmd, "E", 1) == 0)) {
      cmd[strlen(cmd) - 1] = 0;
      char *t = strtok(cmd, " ");
      int i = 0;
      while (t != 0) {
        args[i] = t;
        t = strtok(0, " ");
        i++;
      }
      args[i] = NULL;
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