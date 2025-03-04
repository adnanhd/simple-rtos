#include "memory_alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long Align;  // Force alignment to a long boundary

// Header for each allocated block
union header {
    struct {
        union header *next; // Next free block
        size_t size;        // Size of the block in header units
    } s;
    Align x; // Force alignment of the header.
};

typedef union header Header;

#define HEAP_SIZE (1024 * 1024)  // 1 MB heap

// Our fixed heap buffer and pointers to track allocations.
static char my_heap[HEAP_SIZE];
static char *heap_ptr;         // Points to the next free byte in my_heap
static char *heap_end = my_heap + HEAP_SIZE;

static Header *freep = NULL;   // Start of the free list

// Initialize the memory allocator by placing the free list base within our heap.
void init_allocator(void) {
    freep = (Header *)my_heap;
    freep->s.size = 0;
    freep->s.next = freep;     // Create a circular free list.
    heap_ptr = my_heap + sizeof(Header); // Reserve space for the base header.
}

// Request more memory from the fixed heap buffer.
static Header *my_morecore(size_t nu) {
    size_t nbytes = nu * sizeof(Header);
    if (heap_ptr + nbytes > heap_end) {
        // Not enough memory available.
        return NULL;
    }
    Header *up = (Header *)heap_ptr;
    up->s.size = nu;
    heap_ptr += nbytes;
    // Add the new block to the free list.
    my_free((void *)(up + 1));
    return freep;
}

// Custom malloc: allocate nbytes of memory.
void *my_malloc(size_t nbytes) {
    Header *p, *prevp;
    size_t nunits;
    
    // Initialize the allocator if it's not yet initialized.
    if (freep == NULL) {
        init_allocator();
    }
    
    // Calculate how many header-sized units are needed (include space for header).
    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    
    prevp = freep;
    for (p = prevp->s.next; ; prevp = p, p = p->s.next) {
        if (p->s.size >= nunits) { // Found a block large enough.
            if (p->s.size == nunits) { // Exact fit.
                prevp->s.next = p->s.next;
            } else { // Allocate tail end.
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p + 1);
        }
        if (p == freep) {  // Wrapped around the free list; no block found.
            Header *more = my_morecore(nunits);
            if (more == NULL)
                return NULL; // Out of memory.
        }
    }
}

// Custom free: returns a block to the free list.
void my_free(void *ap) {
    Header *bp, *p;
    bp = (Header *)ap - 1; // Get pointer to block header.
    
    // Find the correct place to insert the free block.
    for (p = freep; !(bp > p && bp < p->s.next); p = p->s.next) {
        // Handle the case when bp is before the first block or after the last block.
        if (p >= p->s.next && (bp > p || bp < p->s.next))
            break;
    }
    
    // Join to the upper neighbor if adjacent.
    if (bp + bp->s.size == p->s.next) {
        bp->s.size += p->s.next->s.size;
        bp->s.next = p->s.next->s.next;
    } else {
        bp->s.next = p->s.next;
    }
    
    // Join to the lower neighbor if adjacent.
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.next = bp->s.next;
    } else {
        p->s.next = bp;
    }
    freep = p;
}

// Custom realloc: resize an allocated block.
void *my_realloc(void *ptr, size_t size) {
    if (ptr == NULL)
        return my_malloc(size);
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }
    
    Header *bp = (Header *)ptr - 1;
    size_t old_size = (bp->s.size - 1) * sizeof(Header);
    void *new_ptr = my_malloc(size);
    if (new_ptr == NULL)
        return NULL;
    size_t copy_size = old_size < size ? old_size : size;
    memcpy(new_ptr, ptr, copy_size);
    my_free(ptr);
    return new_ptr;
}
