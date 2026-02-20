#ifndef RTOS_TIMER_H
#define RTOS_TIMER_H

#include "types.h"

void timer_init(uint32_t interval_us);
void timer_irq_handler(void);

#endif /* RTOS_TIMER_H */
