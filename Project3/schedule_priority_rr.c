/**
 * Priority Round-Robin Scheduling
 * Vinisha Bala Dhayanidhi 
 * 27th February 2025
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schedulers.h"
#include "task.h"
#include "cpu.h"
#include "list.h"

#define QUANTUM 10
#define MAX_PRIORITY 10
#define MIN_PRIORITY 1
#define MAX_TASKS 100

typedef struct
{
    char *name;
    int original_burst;
    int response_time;
    int finish_time;
} TaskStats;

TaskStats stats[MAX_TASKS];
int stats_count = 0;

// global arrays for priority round-robin queues (indices 1..10 used)
struct node *prr_heads[11] = {NULL};
struct node *prr_tails[11] = {NULL};
int time = 0;     // CPU run time
int sim_time = 0; // simulation time including dispatcher overhead
int dispatcher_count = 0;

// returns 1 if any tasks remain in any priority queue
int tasksRemaining()
{
    for (int p = MIN_PRIORITY; p <= MAX_PRIORITY; p++)
    {
        if (prr_heads[p] != NULL)
            return 1;
    }
    return 0;
}

// enqueue a task into its appropriate priority queue.
void enqueue_priority(Task *task)
{
    int p = task->priority; // priority is in [1,10]
    struct node *newNode = malloc(sizeof(struct node));
    newNode->task = task;
    newNode->next = NULL;
    if (prr_heads[p] == NULL)
    {
        prr_heads[p] = prr_tails[p] = newNode;
    }
    else
    {
        prr_tails[p]->next = newNode;
        prr_tails[p] = newNode;
    }
}

// dequeue a task from the given priority queue.
Task *dequeue_priority(int p)
{
    if (prr_heads[p] == NULL)
        return NULL;
    struct node *temp = prr_heads[p];
    Task *task = temp->task;
    prr_heads[p] = prr_heads[p]->next;
    if (prr_heads[p] == NULL)
        prr_tails[p] = NULL;
    free(temp);
    return task;
}

// retrieve bonus stats for a given task.
TaskStats *get_stats(char *name)
{
    for (int i = 0; i < stats_count; i++)
    {
        if (strcmp(stats[i].name, name) == 0)
            return &stats[i];
    }
    return NULL;
}

// comparator for sorting bonus stats.
int cmp_stats(const void *a, const void *b)
{
    TaskStats *ts1 = (TaskStats *)a;
    TaskStats *ts2 = (TaskStats *)b;
    return strcmp(ts1->name, ts2->name);
}

// create a new task, enqueue it in its priority queue, and record bonus stats.
void add(char *name, int priority, int burst, int tid)
{
    Task *newTask = malloc(sizeof(Task));
    newTask->name = strdup(name);
    newTask->priority = priority;
    newTask->burst = burst;
    newTask->tid = 0;
    enqueue_priority(newTask);

    stats[stats_count].name = strdup(name);
    stats[stats_count].original_burst = burst;
    stats[stats_count].response_time = -1;
    stats[stats_count].finish_time = -1;
    stats_count++;
}

// process tasks by priority (highest first) using round robin per queue,
// update bonus stats, and then output bonus information.
void schedule()
{
    // loop from highest priority to lowest.
    for (int p = MAX_PRIORITY; p >= MIN_PRIORITY; p--)
    {
        while (prr_heads[p] != NULL)
        {
            Task *task = dequeue_priority(p);
            TaskStats *ts = get_stats(task->name);
            if (ts->response_time == -1)
                ts->response_time = sim_time;

            int slice = (task->burst > QUANTUM) ? QUANTUM : task->burst;
            run(task, slice);
            time += slice;
            sim_time += slice;
            task->burst -= slice;

            // if any tasks remain in any queue, add dispatcher overhead.
            if (tasksRemaining())
            {
                sim_time += 1;
                dispatcher_count++;
            }

            if (task->burst > 0)
            {
                // not finished; re-enqueue into the same priority queue.
                enqueue_priority(task);
            }
            else
            {
                // finished; record finish time and free task memory.
                ts->finish_time = sim_time;
                free(task->name);
                free(task);
            }
            printf("\tTime is now: %d\n", time);
        }
    }

    // bonus
    printf("\nBonus Output:\n");
    double cpu_util = ((double)time / sim_time) * 100.0;
    printf("CPU Utilization: %.2f%%\n", cpu_util);

    qsort(stats, stats_count, sizeof(TaskStats), cmp_stats);

    printf("       |");
    for (int i = 0; i < stats_count; i++)
    {
        printf(" %s |", stats[i].name);
    }
    printf("\n");

    printf("TAT    |");
    for (int i = 0; i < stats_count; i++)
    {
        printf(" %d |", stats[i].finish_time);
    }
    printf("\n");

    printf("WT     |");
    for (int i = 0; i < stats_count; i++)
    {
        int wt = stats[i].finish_time - stats[i].original_burst;
        printf(" %d |", wt);
    }
    printf("\n");

    printf("RT     |");
    for (int i = 0; i < stats_count; i++)
    {
        printf(" %d |", stats[i].response_time);
    }
    printf("\n");
}