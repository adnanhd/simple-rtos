#ifndef MEM_H
#define MEM_H

#include <stddef.h>

void init_allocator(void);
void *my_malloc(size_t nbytes);
void my_free(void *ap);
void *my_realloc(void *ptr, size_t size);

#endif /* MEM_H */
