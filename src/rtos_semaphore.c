#include "rtos.h"
#include "rtos_kernel.h"
#include <stddef.h>

// Initialize semaphore with an initial value.
void Semaphore_Init(Semaphore_t *sem, int init_val) {
    sem->count = init_val;
    sem->waiting = NULL;
}

// Wait (P) operation: decrement count; if less than zero, block current task.
void Semaphore_Wait(Semaphore_t *sem) {
    sem->count--;
    if (sem->count < 0) {
        // Block the current task.
        currentTask->state = TASK_BLOCKED;
        // Add currentTask to the semaphore waiting list (simple insertion).
        currentTask->next = sem->waiting;
        sem->waiting = currentTask;
        // Yield to another task.
        RTOS_Yield();
    }
}

// Signal (V) operation: increment count; if any tasks are waiting, unblock one.
void Semaphore_Signal(Semaphore_t *sem) {
    sem->count++;
    if (sem->count <= 0) {
        // Unblock one waiting task.
        if (sem->waiting != NULL) {
            TCB_t *taskToUnblock = sem->waiting;
            sem->waiting = sem->waiting->next;
            taskToUnblock->state = TASK_READY;
        }
    }
}
