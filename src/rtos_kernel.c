#include <stdio.h>
#include <stdlib.h>
#include "rtos.h"

// Global variables for task management
TCB_t *taskList = NULL;
TCB_t *currentTask = NULL;
static int taskCount = 0;

void RTOS_Init(void) {
    taskList = NULL;
    currentTask = NULL;
    taskCount = 0;
}

int RTOS_CreateTask(TaskFunction_t task) {
    if (taskCount >= MAX_TASKS) {
        return -1;
    }
    TCB_t *newTask = (TCB_t *)malloc(sizeof(TCB_t));
    if (!newTask) return -1;
    
    // Initialize the task context
    if (getcontext(&newTask->context) == -1) {
        free(newTask);
        return -1;
    }
    newTask->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    if (!newTask->context.uc_stack.ss_sp) {
        free(newTask);
        return -1;
    }
    newTask->context.uc_stack.ss_size = STACK_SIZE;
    newTask->context.uc_link = NULL; // When the task finishes, there's no continuation
    newTask->state = TASK_READY;
    makecontext(&newTask->context, task, 0);
    
    // Add the new task to the circular task list
    if (taskList == NULL) {
        taskList = newTask;
        newTask->next = newTask;
    } else {
        TCB_t *temp = taskList;
        while (temp->next != taskList)
            temp = temp->next;
        temp->next = newTask;
        newTask->next = taskList;
    }
    taskCount++;
    return 0;
}

void RTOS_Yield(void) {
    if (currentTask == NULL) return;
    TCB_t *prevTask = currentTask;
    // Find the next READY task (skip those that are BLOCKED)
    do {
        currentTask = currentTask->next;
    } while (currentTask->state == TASK_BLOCKED && currentTask != prevTask);
    swapcontext(&prevTask->context, &currentTask->context);
}

void RTOS_Start(void) {
    if (taskList == NULL) return;
    currentTask = taskList;
    // Begin execution with the first task
    setcontext(&currentTask->context);
}
