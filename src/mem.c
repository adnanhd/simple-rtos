#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long Align;

union header {
    struct {
        union header *next;
        size_t size;
    } s;
    Align x;
};

typedef union header Header;

#define HEAP_SIZE (1024 * 1024)
static char my_heap[HEAP_SIZE];
static char *heap_ptr;
static char *heap_end = my_heap + HEAP_SIZE;

static Header *freep = NULL;

void init_allocator(void) {
    freep = (Header *)my_heap;
    freep->s.size = 0;
    freep->s.next = freep;
    heap_ptr = my_heap + sizeof(Header);
}

static Header *morecore(size_t nu) {
    size_t nbytes = nu * sizeof(Header);
    if (heap_ptr + nbytes > heap_end)
        return NULL;
    Header *up = (Header *)heap_ptr;
    up->s.size = nu;
    heap_ptr += nbytes;
    my_free((void *)(up + 1));
    return freep;
}

void *my_malloc(size_t nbytes) {
    Header *p, *prevp;
    size_t nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    if (freep == NULL)
        init_allocator();
    prevp = freep;
    for (p = prevp->s.next;; prevp = p, p = p->s.next) {
        if (p->s.size >= nunits) {
            if (p->s.size == nunits)
                prevp->s.next = p->s.next;
            else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p + 1);
        }
        if (p == freep) {
            Header *more = morecore(nunits);
            if (more == NULL)
                return NULL;
        }
    }
}

void my_free(void *ap) {
    Header *bp = (Header *)ap - 1;
    Header *p;
    for (p = freep; !(bp > p && bp < p->s.next); p = p->s.next) {
        if (p >= p->s.next && (bp > p || bp < p->s.next))
            break;
    }
    if (bp + bp->s.size == p->s.next) {
        bp->s.size += p->s.next->s.size;
        bp->s.next = p->s.next->s.next;
    } else {
        bp->s.next = p->s.next;
    }
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.next = bp->s.next;
    } else {
        p->s.next = bp;
    }
    freep = p;
}

void *my_realloc(void *ptr, size_t size) {
    if (!ptr)
        return my_malloc(size);
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }
    Header *bp = (Header *)ptr - 1;
    size_t old_size = (bp->s.size - 1) * sizeof(Header);
    void *new_ptr = my_malloc(size);
    if (!new_ptr)
        return NULL;
    size_t copy_size = old_size < size ? old_size : size;
    memcpy(new_ptr, ptr, copy_size);
    my_free(ptr);
    return new_ptr;
}
