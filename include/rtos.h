#ifndef RTOS_H
#define RTOS_H

#include <ucontext.h>
#include <stdint.h>

#define MAX_TASKS 10
#define STACK_SIZE 16384  // 16KB per task

// Task function type.
typedef void (*TaskFunction_t)(void);

// Task states.
enum { TASK_READY, TASK_BLOCKED };

// Task Control Block (TCB)
typedef struct TCB {
    ucontext_t context;   // Context for task switching.
    struct TCB *next;     // Next task in the circular list.
    int state;            // TASK_READY or TASK_BLOCKED.
    uint32_t delayTicks;  // Number of ticks to delay/sleep.
    void *message;        // Optional storage for IPC/message passing.
} TCB_t;

// Global task pointers.
extern TCB_t *taskList;
extern TCB_t *currentTask;

// Kernel API.
void RTOS_Init(void);
int RTOS_CreateTask(TaskFunction_t task);
void RTOS_Yield(void);
void RTOS_Start(void);
void RTOS_Delay(uint32_t ticks);

// Simple Semaphore API.
typedef struct Semaphore {
    int count;
    TCB_t *waiting;  // Pointer to a waiting tasks list.
} Semaphore_t;

void Semaphore_Init(Semaphore_t *sem, int init_val);
void Semaphore_Wait(Semaphore_t *sem);
void Semaphore_Signal(Semaphore_t *sem);

#endif // RTOS_H
