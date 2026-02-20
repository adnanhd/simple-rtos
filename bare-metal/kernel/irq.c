#include "irq.h"
#include "timer.h"
#include "scheduler.h"
#include "platform.h"

/* BCM2835 interrupt controller */
#define IRQ_BASE            (PERIPHERAL_BASE + 0xB000)
#define IRQ_BASIC_PENDING   (IRQ_BASE + 0x200)

/* ARM inline assembly IRQ control */
uint32_t irq_disable(void) {
    uint32_t cpsr;
    __asm__ volatile("mrs %0, cpsr" : "=r"(cpsr));
    __asm__ volatile("cpsid i");
    return cpsr;
}

void irq_restore(uint32_t flags) {
    __asm__ volatile("msr cpsr_c, %0" : : "r"(flags));
}

void irq_enable(void) {
    __asm__ volatile("cpsie i");
}

/* Context switch declared in assembly */
extern void context_switch(uint32_t **old_sp, uint32_t *new_sp);

/* Called from vectors.s IRQ stub */
void irq_dispatch(void) {
    uint32_t pending = mmio_read(IRQ_BASIC_PENDING);

    /* Bit 0: ARM Timer interrupt */
    if (pending & (1 << 0)) {
        timer_irq_handler();
        scheduler_tick();

        /* If current task was preempted (time slice expired), switch */
        if (current_tcb && current_tcb->state != TASK_STATE_RUNNING) {
            TCB_t *old = current_tcb;
            TCB_t *next = scheduler_select_next();
            if (next != old) {
                context_switch(&old->sp, next->sp);
            }
        }
    }
}
