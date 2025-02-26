#include "cpu.h"
#include "list.h"
#include "schedulers.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIME_QUANTUM 10

struct node *g_head = NULL; // Global task list

// Comparator function for priority order
int compareTasks(const void *a, const void *b) {
  Task *taskA = *(Task **)a;
  Task *taskB = *(Task **)b;
  return taskB->priority - taskA->priority; // Higher priority first
}

// Function to sort tasks by priority
void sortTaskList() {
  int count = 0;
  struct node *temp = g_head;
  while (temp) {
    count++;
    temp = temp->next;
  }

  if (count < 2)
    return; // No need to sort if 0 or 1 task

  Task *taskArray[count];
  temp = g_head;
  for (int i = 0; i < count; i++) {
    taskArray[i] = temp->task;
    temp = temp->next;
  }

  qsort(taskArray, count, sizeof(Task *), compareTasks);

  // Rebuild the linked list with sorted tasks
  g_head = NULL;
  for (int i = 0; i < count; i++) {
    insert(&g_head, taskArray[i]);
  }
}

// Add a task to the queue
void add(char *name, int priority, int burst) {
  Task *newTask = (Task *)malloc(sizeof(Task));
  newTask->name = strdup(name);
  newTask->priority = priority;
  newTask->burst = burst;
  insert(&g_head, newTask);
}

// Picks the next task without removing it permanently
Task *pickNextTask() {
  if (!g_head)
    return NULL;
  return g_head->task;
}

// Moves the head task to the back (corrects RR order)
void moveToBack() {
  if (!g_head || !g_head->next)
    return;

  struct node *temp = g_head;
  g_head = g_head->next; // Move head forward
  temp->next = NULL;

  struct node *last = g_head;
  while (last->next)
    last = last->next;
  last->next = temp;
}

// Schedules tasks using Round Robin in priority order
void schedule() {
  sortTaskList(); // First, sort the tasks by priority

  int currentTime = 0;
  int totalBurstTime = 0;
  int dispatcherTime = 0;
  int numTasks = 0;

  char *taskNames[100];
  int burstTimes[100];
  int startTimes[100] = {0}; // Initialize start times
  int finishTimes[100];

  while (g_head != NULL) {
    Task *task = pickNextTask(); // Get the next task
    if (!task)
      break;

    if (startTimes[numTasks] == 0) {
      taskNames[numTasks] = task->name;
      burstTimes[numTasks] = task->burst;
      startTimes[numTasks] = currentTime;
    }

    int timeSlice = (task->burst > TIME_QUANTUM) ? TIME_QUANTUM : task->burst;
    run(task, timeSlice);
    currentTime += timeSlice;
    task->burst -= timeSlice;

    printf(" Time: %d\n", currentTime);

    if (task->burst > 0) {
      moveToBack(); // Move unfinished task to the end
    } else {
      finishTimes[numTasks] = currentTime;
      totalBurstTime += burstTimes[numTasks];

      struct node *temp = g_head;
      g_head = g_head->next;
      free(temp->task->name);
      free(temp->task);
      free(temp);

      numTasks++;
    }

    if (g_head != NULL)
      dispatcherTime++;
  }

  int totalTime = totalBurstTime + dispatcherTime;
  printf("Total Time required is %d units\n", totalTime);
  double cpuUtilization = ((double)totalBurstTime / totalTime) * 100.0;
  printf("CPU Utilization: %.2f%%\n", cpuUtilization);

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