#ifndef RTOS_KPRINTF_H
#define RTOS_KPRINTF_H

#include "types.h"

typedef void (*kputc_fn)(char c);

void kprintf_init(kputc_fn putc);
void kprintf(const char *fmt, ...);

#endif /* RTOS_KPRINTF_H */
