// Yu Dinh - Project 3 Scheduling Algorithms
// schedule_rr.c

#include "cpu.h"
#include "list.h"
#include "schedulers.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUANTUM 10
#define MAX_TASKS 100

typedef struct {
  char *name;
  int original_burst;
  int response_time;
  int finish_time;
} TaskStats;

TaskStats stats[MAX_TASKS];
int stats_count = 0;

// global variables for the Round Robin queue.
struct node *rr_head = NULL;
struct node *rr_tail = NULL;
int time = 0;     // CPU run time
int sim_time = 0; // simulation time including dispatcher overhead
int dispatcher_count = 0;

// enqueue a task into the RR queue.
void enqueue(Task *task) {
  struct node *newNode = malloc(sizeof(struct node));
  newNode->task = task;
  newNode->next = NULL;
  if (rr_tail == NULL) {
    rr_head = rr_tail = newNode;
  } else {
    rr_tail->next = newNode;
    rr_tail = newNode;
  }
}

// dequeue a task from the RR queue.
Task *dequeue() {
  if (rr_head == NULL)
    return NULL;
  struct node *temp = rr_head;
  Task *task = temp->task;
  rr_head = rr_head->next;
  if (rr_head == NULL)
    rr_tail = NULL;
  free(temp);
  return task;
}

// retrieve bonus stats for a given task.
TaskStats *get_stats(char *name) {
  for (int i = 0; i < stats_count; i++) {
    if (strcmp(stats[i].name, name) == 0)
      return &stats[i];
  }
  return NULL;
}

// comparator for sorting bonus stats.
int cmp_stats(const void *a, const void *b) {
  TaskStats *ts1 = (TaskStats *)a;
  TaskStats *ts2 = (TaskStats *)b;
  return strcmp(ts1->name, ts2->name);
}

// enqueue a new task and record its bonus stats.
void add(char *name, int priority, int burst) {
  Task *newTask = malloc(sizeof(Task));
  newTask->name = strdup(name);
  newTask->priority = priority;
  newTask->burst = burst;
  newTask->tid = 0;
  enqueue(newTask);

  stats[stats_count].name = strdup(name);
  stats[stats_count].original_burst = burst;
  stats[stats_count].response_time = -1;
  stats[stats_count].finish_time = -1;
  stats_count++;
}

// run tasks in RR order, update bonus stats per slice, and then output bonus
// info.
void schedule() {
  Task *task;
  while ((task = dequeue()) != NULL) {
    TaskStats *ts = get_stats(task->name);
    if (ts->response_time == -1)
      ts->response_time = sim_time;

    int slice = (task->burst > QUANTUM) ? QUANTUM : task->burst;
    run(task, slice);
    time += slice;
    sim_time += slice;
    task->burst -= slice;

    // if there are more tasks remaining (or this task needs more time), add
    // dispatcher overhead.
    if (rr_head != NULL || task->burst > 0) {
      sim_time += 1;
      dispatcher_count++;
    }

    if (task->burst > 0) {
      // task not finished; re-enqueue.
      enqueue(task);
    } else {
      // task finished; record finish time.
      ts->finish_time = sim_time;
      free(task->name);
      free(task);
    }
    printf("\tTime is now: %d\n", time);
  }

  // bonus
  printf("\nBonus Output:\n");
  double cpu_util = ((double)time / sim_time) * 100.0;
  printf("CPU Utilization: %.2f%%\n", cpu_util);

  qsort(stats, stats_count, sizeof(TaskStats), cmp_stats);

  printf("       |");
  for (int i = 0; i < stats_count; i++) {
    printf(" %s |", stats[i].name);
  }
  printf("\n");

  printf("TAT    |");
  for (int i = 0; i < stats_count; i++) {
    printf(" %d |", stats[i].finish_time);
  }
  printf("\n");

  printf("WT     |");
  for (int i = 0; i < stats_count; i++) {
    int wt = stats[i].finish_time - stats[i].original_burst;
    printf(" %d |", wt);
  }
  printf("\n");

  printf("RT     |");
  for (int i = 0; i < stats_count; i++) {
    printf(" %d |", stats[i].response_time);
  }
  printf("\n");
}