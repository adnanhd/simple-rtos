#include "rtos.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

// Global task pointers.
TCB_t *taskList = NULL;
TCB_t *currentTask = NULL;
static int taskCount = 0;

// Idle task definition.
static void idleTask(void) {
    while (1) {
        // The idle task simply yields control, allowing delay ticks to be decremented.
        RTOS_Yield();
    }
}

// Internal function to simulate a tick (decrement delay counters).
static void internal_RTOS_Tick(void) {
    if (taskList == NULL) return;
    TCB_t *temp = taskList;
    do {
        if (temp->delayTicks > 0)
            temp->delayTicks--;
        temp = temp->next;
    } while (temp != taskList);
}

void RTOS_Init(void) {
    taskList = NULL;
    currentTask = NULL;
    taskCount = 0;
    // Create the idle task first.
    RTOS_CreateTask(idleTask);
}

int RTOS_CreateTask(TaskFunction_t task) {
    if (taskCount >= MAX_TASKS)
        return -1;
    TCB_t *newTask = (TCB_t *)malloc(sizeof(TCB_t));
    if (!newTask)
        return -1;
    
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
    newTask->context.uc_link = NULL;
    newTask->state = TASK_READY;
    newTask->delayTicks = 0;
    newTask->message = NULL;
    
    makecontext(&newTask->context, task, 0);
    
    // Insert new task into circular linked list.
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
    TCB_t *prevTask = currentTask;
    internal_RTOS_Tick(); // Decrement delay counters.
    do {
        currentTask = currentTask->next;
    } while (currentTask->delayTicks > 0 || currentTask->state == TASK_BLOCKED);
    swapcontext(&prevTask->context, &currentTask->context);
}

void RTOS_Delay(uint32_t ticks) {
    if (currentTask != NULL) {
        currentTask->delayTicks = ticks;
        RTOS_Yield();
    }
}

void RTOS_Start(void) {
    if (taskList == NULL)
        return;
    currentTask = taskList;
    setcontext(&currentTask->context);
}
