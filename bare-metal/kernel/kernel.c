#include "kernel.h"
#include "scheduler.h"
#include "uart.h"
#include "timer.h"
#include "mem.h"
#include "kprintf.h"
#include "irq.h"

extern void context_switch(uint32_t **old_sp, uint32_t *new_sp);
extern void task_initial_entry(void);

/* Declared in irq.h */
extern void irq_enable(void);

static void task_init_stack(TCB_t *tcb, TaskFunction_t func) {
    uint32_t *sp = tcb->stack_base + (TASK_STACK_SIZE / sizeof(uint32_t));

    /* Build a fake context frame matching context_switch's layout:
     * push {cpsr}, push {r0-r12, lr} */
    *(--sp) = 0x00000153;                    /* cpsr: SVC mode, IRQs enabled */
    *(--sp) = (uint32_t)task_initial_entry;  /* lr */
    *(--sp) = 0;                             /* r12 */
    *(--sp) = 0;                             /* r11 */
    *(--sp) = 0;                             /* r10 */
    *(--sp) = 0;                             /* r9 */
    *(--sp) = 0;                             /* r8 */
    *(--sp) = 0;                             /* r7 */
    *(--sp) = 0;                             /* r6 */
    *(--sp) = 0;                             /* r5 */
    *(--sp) = (uint32_t)func;               /* r4 = task function pointer */
    *(--sp) = 0;                             /* r3 */
    *(--sp) = 0;                             /* r2 */
    *(--sp) = 0;                             /* r1 */
    *(--sp) = 0;                             /* r0 */

    tcb->sp = sp;
}

extern TCB_t *scheduler_get_task_pool(void);

int task_create(TaskFunction_t func, uint8_t priority) {
    if (priority >= MAX_PRIORITIES)
        return -1;

    TCB_t *tcb = scheduler_alloc_task();
    if (!tcb)
        return -1;

    tcb->priority = priority;
    tcb->delay_ticks = 0;
    tcb->message = NULL;

    /* Allocate task stack */
    tcb->stack_base = (uint32_t *)my_malloc(TASK_STACK_SIZE);
    if (!tcb->stack_base) {
        tcb->state = TASK_STATE_DEAD;
        return -1;
    }

    task_init_stack(tcb, func);
    scheduler_add_task(tcb);
    return (int)tcb->task_id;
}

void task_yield(void) {
    uint32_t flags = irq_disable();
    TCB_t *old = current_tcb;
    if (old->state == TASK_STATE_RUNNING) {
        old->state = TASK_STATE_READY;
        scheduler_add_task(old);
    }
    TCB_t *next = scheduler_select_next();
    if (next != old) {
        irq_restore(flags);
        context_switch(&old->sp, next->sp);
    } else {
        irq_restore(flags);
    }
}

void task_sleep(uint32_t ticks) {
    uint32_t flags = irq_disable();
    current_tcb->state = TASK_STATE_SLEEPING;
    current_tcb->delay_ticks = ticks;
    irq_restore(flags);
    task_yield();
}

void task_exit(void) {
    uint32_t flags = irq_disable();
    current_tcb->state = TASK_STATE_DEAD;
    irq_restore(flags);
    task_yield();
    /* Should never reach here */
    while (1) ;
}

/* Declared in scheduler.c, needed here */
extern TCB_t *scheduler_alloc_task(void);
