#include "semaphore.h"
#include "scheduler.h"
#include "irq.h"

/* Weak symbol — overridden by real implementation on bare-metal */
__attribute__((weak)) void task_yield(void) { }

void semaphore_init(Semaphore_t *sem, int init_val) {
    sem->count = init_val;
    sem->waiting = NULL;
}

void semaphore_wait(Semaphore_t *sem) {
    uint32_t flags = irq_disable();
    sem->count--;
    if (sem->count < 0) {
        current_tcb->state = TASK_STATE_BLOCKED;
        current_tcb->next = sem->waiting;
        sem->waiting = current_tcb;
        irq_restore(flags);
        task_yield();
    } else {
        irq_restore(flags);
    }
}

void semaphore_signal(Semaphore_t *sem) {
    uint32_t flags = irq_disable();
    sem->count++;
    if (sem->waiting != NULL) {
        TCB_t *task = sem->waiting;
        sem->waiting = task->next;
        task->next = NULL;
        task->state = TASK_STATE_READY;
        scheduler_add_task(task);
    }
    irq_restore(flags);
}
