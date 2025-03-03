/**
 * Shortest Job First (SJF) Scheduling Algorithm
 * Vinisha Bala Dhayanidhi 
 * 27th February 2025
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

// Global variables to manage the linked list of tasks
struct node *head = NULL;
struct node *tail = NULL;
struct node *temp = NULL;
int count = 0;

// Function to add a task to the list using the shortest job first algorithm.
// The list is sorted by burst time where the shortest burst time is first.
// Insert the tasks sorted by burst time. If two tasks have the same burst time,
// then the task with the highest priority is first. If the priority is the same, sort by TID.
void add(char *name, int priority, int burst, int tid) {
    struct node* curr = head;
    if (!head)  {
        // If the list is empty, initialize the head and tail
        head = malloc(sizeof(struct node));
        head->task = malloc(sizeof(Task));
        head->task->name = name;
        head->task->priority = priority;
        head->task->burst = burst;
        head->task->tid = tid;
        head->next = NULL;
        tail = head;
        count++;
        return;
    }

    // Compare the head burst time to the new burst time
    if (head->task->burst > burst) {
        temp = malloc(sizeof(struct node));
        temp->task = malloc(sizeof(Task));
        temp->task->name = name;
        temp->task->priority = priority;
        temp->task->burst = burst;
        temp->task->tid = tid;
        temp->next = head;
        head = temp;
        count++;
        return;
    }

    // Traverse the list to find the correct position to insert the new task
    while (curr->next && curr->next->task->burst < burst) {
        curr = curr->next;
    }

    // If the burst time is the same, then compare the priority
    if (curr->next && curr->next->task->burst == burst) {
        while (curr->next && curr->next->task->priority < priority && curr->next->task->burst == burst) {
            curr = curr->next;
        }
    }

    // If priority is the same, then compare the TID
    if (curr->next && curr->next->task->priority == priority) {
        while (curr->next && curr->next->task->tid < tid) {
            curr = curr->next;
        }
    }

    // Insert the new task at the correct position
    temp = malloc(sizeof(struct node));
    temp->task = malloc(sizeof(Task));
    temp->task->name = name;
    temp->task->priority = priority;
    temp->task->burst = burst;
    temp->task->tid = tid;
    temp->next = curr->next;
    curr->next = temp;
    count++;
}

// Schedule function that keeps track of time, turnaround time, wait time, and response time
void schedule() {
    // Array to store process values
    struct values process[count + 1];
    double time = 0;
    int tid;

    struct node *curr = head;
    // Iterate through the linked list and run each task
    while (curr)    {
        tid = curr->task->tid;
        
        // Run the current task and update the time
        run(curr->task, curr->task->burst);
        time += curr->task->burst;
        process[tid].turnaround = time;
        process[tid].response = time - curr->task->burst;
        process[tid].wait = time - curr->task->burst;
        curr = curr->next;
        
        printf("Time is now: %0.0lf\n", time);
    }

    // Calculate CPU utilization
    float CPU = (float)(time/(time + (count -1)));
    CPU *= 100;
    printf("CPU Utilization: %0.2f%%\n", CPU);

    // Print time per process
    printf("...|");
    for (int i = 1; i <= count; i++) {
        printf(" T%d |", i);
    }
    printf("\n");

    // Print turnaround time for each process
    printf("TAT|");
    for (int i = 1; i <= count; i++) {
        printf(" %d |", process[i].turnaround);
    }
    printf("\n");

    // Print waiting time for each process
    printf("WT |");
    for (int i = 1; i <= count; i++) {
        printf(" %d |", process[i].wait);
    }
    printf("\n");

    // Print response time for each process
    printf("RT |");
    for (int i = 1; i <= count; i++) {
        printf(" %d |", process[i].response);
    }
    printf("\n");
}