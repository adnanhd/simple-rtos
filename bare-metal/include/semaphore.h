#ifndef RTOS_SEMAPHORE_H
#define RTOS_SEMAPHORE_H

#include "kernel.h"

typedef struct {
    int count;
    TCB_t *waiting;
} Semaphore_t;

void semaphore_init(Semaphore_t *sem, int init_val);
void semaphore_wait(Semaphore_t *sem);
void semaphore_signal(Semaphore_t *sem);

#endif /* RTOS_SEMAPHORE_H */
