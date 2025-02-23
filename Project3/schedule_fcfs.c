#include "cpu.h"
#include "list.h"
#include "schedulers.h"
#include "task.h"
#include <stdbool.h> // For bool
#include <stdio.h>
#include <stdlib.h>         // For malloc and free
#include <string.h>         // For strdup
struct node *g_head = NULL; // Global head of the task list

// Function to add a task to the list
void add(char *name, int priority, int burst) {
  Task *newTask = (Task *)malloc(sizeof(Task));
  newTask->name = strdup(name);
  newTask->priority = priority;
  newTask->burst = burst;
  insert(&g_head, newTask);
}

// Function to compare two task names lexicographically
bool comesBefore(char *a, char *b) { return strcmp(a, b) < 0; }

// Function to pick the next task based on FCFS with lexicographical order
Task *pickNextTask() {
  if (!g_head)
    return NULL;

  struct node *temp = g_head;
  Task *best_sofar = temp->task;

  while (temp != NULL) {
    if (comesBefore(temp->task->name, best_sofar->name))
      best_sofar = temp->task;
    temp = temp->next;
  }

  // Remove the task from the list
  delete (&g_head, best_sofar);
  return best_sofar;
}

// Function to schedule tasks based on FCFS
void schedule() {
  int currentTime = 0;
  Task *task;

  while ((task = pickNextTask()) != NULL) {
    run(task, task->burst);
    currentTime += task->burst;
    printf("Running task = [%s] [%d] [%d] for %d units.\n", task->name,
           task->priority, task->burst, task->burst);
    printf("        Time is now: %d\n", currentTime);
    free(task->name);
    free(task);
  }
}