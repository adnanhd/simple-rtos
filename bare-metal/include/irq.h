#ifndef RTOS_IRQ_H
#define RTOS_IRQ_H

#include "types.h"

/*
 * On bare-metal ARM these use inline asm (cpsid i / msr cpsr_c).
 * For host unit tests, they are no-ops provided as weak symbols.
 */
uint32_t irq_disable(void);
void irq_restore(uint32_t flags);

#endif /* RTOS_IRQ_H */
