#include "cpu.h"
#include "list.h"
#include "schedulers.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIME_QUANTUM 10

struct node *g_head = NULL; // Global head of the task list

// Function to add a task to the list
void add(char *name, int priority, int burst) {
  Task *newTask = (Task *)malloc(sizeof(Task));
  newTask->name = strdup(name);
  newTask->priority = priority;
  newTask->burst = burst;
  insert(&g_head, newTask);
}

// Function to compare two tasks based on priority and lexicographical order
bool comesBefore(Task *a, Task *b) {
  // First, compare based on priority
  if (a->priority > b->priority) {
    return true;
  } else if (a->priority < b->priority) {
    return false;
  }

  // If the priorities are the same, compare lexicographically by name
  return strcmp(a->name, b->name) < 0;
}

// Function to pick the next task based on Priority RR
Task *pickNextTask() {
  if (!g_head)
    return NULL;

  struct node *temp = g_head;
  Task *highestPriorityTask = temp->task;

  // Find the highest priority task and use lexicographical order if priorities
  // are the same
  while (temp != NULL) {
    if (comesBefore(temp->task, highestPriorityTask)) {
      highestPriorityTask = temp->task;
    }
    temp = temp->next;
  }

  // Remove the task from the list
  delete (&g_head, highestPriorityTask);

  return highestPriorityTask;
}

// Function to schedule tasks based on Priority RR
void schedule() {
  int currentTime = 0;
  int totalBurstTime = 0;
  int dispatcherTime = 0;
  int numTasks = 0;
  Task *task;

  // Arrays to store task information for TAT, WT, RT
  char *taskNames[100];
  int burstTimes[100];
  int startTimes[100] = {0}; // Initialize start times
  int finishTimes[100];

  // Add tasks to the list and perform Round Robin Scheduling
  while (g_head != NULL) {
    task = pickNextTask();
    taskNames[numTasks] = task->name;
    burstTimes[numTasks] = task->burst;
    startTimes[numTasks] = currentTime;

    // Round-Robin: Run the task for a time slice or until completion
    int timeSlice = (task->burst > TIME_QUANTUM) ? TIME_QUANTUM : task->burst;
    run(task, timeSlice);
    currentTime += timeSlice;
    task->burst -= timeSlice;

    printf("Running task = [%s] for %d units. Time: %d\n", task->name,
           timeSlice, currentTime);

    if (task->burst > 0) {
      // Reinsert the task into the list in the correct order
      insert(&g_head, task);
    } else {
      finishTimes[numTasks] = currentTime;
      totalBurstTime += burstTimes[numTasks];
      free(task->name);
      free(task);
      numTasks++;
    }

    if (g_head !=
        NULL) { // Increment dispatcher time only if there is a next task
      dispatcherTime++;
    }
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