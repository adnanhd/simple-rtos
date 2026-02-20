#ifndef RTOS_SCHEDULER_H
#define RTOS_SCHEDULER_H

#include "kernel.h"

void scheduler_init(void);
void scheduler_add_task(TCB_t *tcb);
void scheduler_remove_task(TCB_t *tcb);
TCB_t *scheduler_select_next(void);
void scheduler_tick(void);

extern TCB_t *current_tcb;

#endif /* RTOS_SCHEDULER_H */
