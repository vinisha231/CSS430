#include "cpu.h"
#include "list.h"
#include "schedulers.h"
#include "task.h"
#include <stdio.h>

struct node *g_head = NULL; // Global head of the task list

// Function to add a task to the list
void add(char *name, int priority, int burst) {
  Task *newTask = (Task *)malloc(sizeof(Task));
  newTask->name = strdup(name);
  newTask->priority = priority;
  newTask->burst = burst;
  insert(&g_head, newTask);
}

// Function to pick the next task based on FCFS
Task *pickNextTask() {
  if (!g_head)
    return NULL;

  struct node *temp = g_head;
  Task *nextTask = temp->task;

  // Remove the task from the list
  g_head = g_head->next;
  free(temp);

  return nextTask;
}

// Function to schedule tasks based on FCFS
void schedule() {
  int currentTime = 0;
  Task *task;

  while ((task = pickNextTask()) != NULL) {
    run(task, task->burst);
    currentTime += task->burst;
    printf("        Time is now: %d\n", currentTime);
    free(task->name);
    free(task);
  }
}