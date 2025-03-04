#ifndef RTOS_H
#define RTOS_H

#include <ucontext.h>
#include <stdint.h>
#include "callback_mq.h"  // For CallbackMessageQueue

#define MAX_TASKS  10
#define STACK_SIZE 16384  // 16KB per task

typedef void (*TaskFunction_t)(void);

enum { TASK_READY, TASK_BLOCKED };

typedef struct TCB {
    ucontext_t context;
    struct TCB *next;
    int state;            // TASK_READY or TASK_BLOCKED
    uint32_t delayTicks;  // For RTOS_Delay (sleep)
    void *message;        // Holds a received message (if any)
} TCB_t;

// Define a simple semaphore for inter-task synchronization.
typedef struct Semaphore {
    int count;
    TCB_t *waiting; // Pointer to a waiting tasks list (if needed)
} Semaphore_t;

// Global task pointers.
extern TCB_t *taskList;
extern TCB_t *currentTask;

// Kernel API.
void RTOS_Init(void);
int RTOS_CreateTask(TaskFunction_t task);
void RTOS_Yield(void);
void RTOS_Start(void);
void RTOS_Delay(uint32_t ticks);

// Message waiting API.
void RTOS_WaitMessage(CallbackMessageQueue *queue);
void *RTOS_GetMessage(void);

// Semaphore API.
void Semaphore_Init(Semaphore_t *sem, int init_val);
void Semaphore_Wait(Semaphore_t *sem);
void Semaphore_Signal(Semaphore_t *sem);

#endif // RTOS_H
