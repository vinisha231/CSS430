//
//  schedule_priority.c
//  CSS 430
//
//  Created by Uyen Minh Hoang on 4/22/20.
//  Copyright © 2020 Uyen Minh Hoang. All rights reserved.
//

#include "cpu.h"
#include "list.h"
#include "schedulers.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node *list = NULL; // create a task list

void add(char *name, int priority, int burst) {
  // allocate a space for the struct
  Task *newTask = malloc(sizeof(Task)); // same way the source code use

  // initialize the new node
  newTask->name = name;
  newTask->priority = priority;
  newTask->burst = burst;
  insert(&list, newTask);
}

// need to find the task with the shortest burst time
Task *pickNextTask() { // have to put the method as a pointer to use it in the
                       // next method
  struct node *currentTask = list;
  Task *priorityTask = list->task;
  while (currentTask != NULL) {
    // add "=" sign so when there are tasks with the same priority, the tasks
    // will be picked by the order they arrive
    if (currentTask->task->priority > priorityTask->priority) {
      priorityTask = currentTask->task;
    }
    if (currentTask->task->priority == priorityTask->priority) {
      if (strcmp(currentTask->task->name, priorityTask->name) < 0) {
        priorityTask = currentTask->task;
      }
    }
    currentTask = currentTask->next;
  }
  return priorityTask;
}

void schedule() {
  while (list != NULL) {
    Task *currentTask = pickNextTask(); // need pointer to access the struct
    run(currentTask, currentTask->burst);
    delete (&list, currentTask);
  }
}