#include "cpu.h"
#include "list.h"
#include "schedulers.h"
#include "task.h"
#include <stdbool.h> // For bool
#include <stdio.h>
#include <stdlib.h> // For malloc and free
#include <string.h> // For strdup

struct node *g_head = NULL; // Global head of the task list

// Function to add a task to the list
void add(char *name, int priority, int burst) {
  Task *newTask = (Task *)malloc(sizeof(Task));
  newTask->name = strdup(name);
  newTask->priority = priority;
  newTask->burst = burst;
  insert(&g_head, newTask);
}

// Function to pick the next task based on SJF
Task *pickNextTask() {
  if (!g_head)
    return NULL;

  struct node *temp = g_head;
  Task *shortestTask = temp->task;

  while (temp != NULL) {
    if (temp->task->burst < shortestTask->burst)
      shortestTask = temp->task;
    temp = temp->next;
  }

  // Remove the task from the list
  delete (&g_head, shortestTask);
  return shortestTask;
}

// Function to schedule tasks based on SJF
void schedule() {
  int currentTime = 0;
  int totalBurstTime = 0;
  int dispatcherTime = 0;
  int numTasks = 0;
  Task *task;

  // Arrays to store task information for TAT, WT, RT
  char *taskNames[100];
  int burstTimes[100];
  int startTimes[100];
  int finishTimes[100];

  while ((task = pickNextTask()) != NULL) {
    taskNames[numTasks] = task->name;
    burstTimes[numTasks] = task->burst;
    startTimes[numTasks] = currentTime;
    run(task, task->burst);
    currentTime += task->burst;
    finishTimes[numTasks] = currentTime;
    totalBurstTime += task->burst;
    if (g_head !=
        NULL) { // Increment dispatcher time only if there is a next task
      dispatcherTime++;
    }
    printf("        Time is now: %d\n", currentTime);
    free(task->name);
    free(task);
    numTasks++;
  }

  // Calculate CPU utilization
  int totalTime = totalBurstTime + dispatcherTime;
  printf("Total Time required is %d units\n", totalTime);
  double cpuUtilization = ((double)totalBurstTime / totalTime) * 100.0;
  printf("CPU Utilization: %.2f%%\n", cpuUtilization);

  // Print TAT, WT, RT table
  printf("...|");
  for (int i = 0; i < numTasks; i++) {
    printf(" %s |", taskNames[i]);
  }
  printf("\nTAT|");
  for (int i = 0; i < numTasks; i++) {
    printf(" %d |", finishTimes[i]);
  }
  printf("\nWT |");
  for (int i = 0; i < numTasks; i++) {
    printf(" %d |", startTimes[i]);
  }
  printf("\nRT |");
  for (int i = 0; i < numTasks; i++) {
    printf(" %d |", startTimes[i]);
  }
  printf("\n");
}