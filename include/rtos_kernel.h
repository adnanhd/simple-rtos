#include <stdio.h>
#include <stdlib.h>
#include "rtos.h"

// Global variables for task management
TCB_t *taskList = NULL;
TCB_t *currentTask = NULL;
static int taskCount = 0;

// Forward declaration of the idle task.
static void idleTask(void);

void RTOS_Init(void) {
    taskList = NULL;
    currentTask = NULL;
    taskCount = 0;
    // Create the idle task so that there is always one ready to run.
    RTOS_CreateTask(idleTask);
}

int RTOS_CreateTask(TaskFunction_t task) {
    if (taskCount >= MAX_TASKS) {
        return -1;
    }
    TCB_t *newTask = (TCB_t *)malloc(sizeof(TCB_t));
    if (!newTask) return -1;
    
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
    newTask->context.uc_link = NULL; // No continuation when task ends.
    newTask->state = TASK_READY;
    newTask->delayTicks = 0;
    newTask->message = NULL;
    makecontext(&newTask->context, task, 0);
    
    // Insert new task into the circular linked list.
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

// RTOS_Tick simulates a timer tick by decrementing delay counters.
static void RTOS_Tick(void) {
    if (taskList == NULL) return;
    TCB_t *temp = taskList;
    do {
        if (temp->delayTicks > 0) {
            temp->delayTicks--;
        }
        temp = temp->next;
    } while (temp != taskList);
}

// The scheduler: yields control to the next ready task.
void RTOS_Yield(void) {
    TCB_t *prevTask = currentTask;
    RTOS_Tick();  // Simulate a tick on each yield.
    do {
        currentTask = currentTask->next;
    } while (currentTask->delayTicks > 0 || currentTask->state == TASK_BLOCKED);
    swapcontext(&prevTask->context, &currentTask->context);
}

// RTOS_Delay sets a delay (in ticks) for the current task and yields.
void RTOS_Delay(uint32_t ticks) {
    currentTask->delayTicks = ticks;
    RTOS_Yield();
}

// The idle task simply yields in an infinite loop.
static void idleTask(void) {
    while (1) {
        RTOS_Yield();
    }
}

// --- New Section: Callback Message Waiting Integration --- //

// This callback is invoked when a message is published to the queue.
// The context parameter is the TCB of the waiting task.
static void rtos_message_callback(void *msg, void *context) {
    TCB_t *tcb = (TCB_t *)context;
    tcb->message = msg;
    tcb->state = TASK_READY;
}

// RTOS_WaitMessage causes the current task to block until a message is published
// on the given callback-based message queue.
void RTOS_WaitMessage(CallbackMessageQueue *queue) {
    // Block the task.
    currentTask->state = TASK_BLOCKED;
    // Subscribe our callback, passing currentTask as context.
    callback_mq_subscribe(queue, rtos_message_callback, currentTask);
    // Yield control. When a message is published, our callback will set state to READY.
    RTOS_Yield();
    // Unsubscribe to avoid duplicate callbacks.
    callback_mq_unsubscribe(queue, rtos_message_callback, currentTask);
}

// Retrieve the received message (stored in currentTask->message).
void *RTOS_GetMessage(void) {
    return currentTask->message;
}

// RTOS_Start begins scheduling by setting context of the first task.
void RTOS_Start(void) {
    if (taskList == NULL) return;
    currentTask = taskList;
    setcontext(&currentTask->context);
}
