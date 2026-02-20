#ifndef RTOS_MEM_H
#define RTOS_MEM_H

#include "types.h"

void init_allocator(void);
void *my_malloc(size_t nbytes);
void my_free(void *ap);
void *my_realloc(void *ptr, size_t size);

#endif /* RTOS_MEM_H */
