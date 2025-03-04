#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ==================================
   Simple Memory Allocator (K&R Style)
   ==================================
   This implementation uses a fixed-size heap buffer.
   We initialize the free list from within the heap.
*/

typedef long Align;  // Force alignment to long boundary

// Header for each allocated block.
union header {
    struct {
        union header *next; // Next free block
        size_t size;        // Size of the block in header units
    } s;
    Align x; // Force alignment of the header.
};

typedef union header Header;

#define HEAP_SIZE (1024 * 1024)  // 1 MB heap
static char my_heap[HEAP_SIZE];
static char *heap_ptr;         // Next free position in my_heap
static char *heap_end = my_heap + HEAP_SIZE;

static Header *freep = NULL;   // Start of free list

// Initialize the memory allocator. This places the free list's base
// inside our heap, ensuring all blocks are in a contiguous address range.
void init_allocator(void) {
    freep = (Header*)my_heap; 
    freep->s.size = 0;       
    freep->s.next = freep;   // Circular free list.
    heap_ptr = my_heap + sizeof(Header); // Reserve space for the base block.
}

// Custom free: return a block to the free list.
void my_free(void *ap) {
    Header *bp, *p;
    bp = (Header *)ap - 1; // Point to block header.
    // Find the correct place to insert the free block.
    for (p = freep; !(bp > p && bp < p->s.next); p = p->s.next) {
        // Handle the case where bp is before the first block or after the last block.
        if (p >= p->s.next && (bp > p || bp < p->s.next))
            break;
    }
    // Join to upper neighbor if adjacent.
    if (bp + bp->s.size == p->s.next) {
        bp->s.size += p->s.next->s.size;
        bp->s.next = p->s.next->s.next;
    } else {
        bp->s.next = p->s.next;
    }
    // Join to lower neighbor if adjacent.
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.next = bp->s.next;
    } else {
        p->s.next = bp;
    }
    freep = p;
}

// Request more memory from our fixed heap buffer.
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
    
    // Initialize the allocator if not already done.
    if (freep == NULL) {
        init_allocator();
    }
    
    // Calculate how many header-sized units are needed (including header).
    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    
    prevp = freep;
    // Traverse the free list looking for a block that fits.
    for (p = prevp->s.next;; prevp = p, p = p->s.next) {
        if (p->s.size >= nunits) { // Found a block large enough.
            if (p->s.size == nunits) { // Exact fit.
                prevp->s.next = p->s.next;
            } else { // Allocate tail end.
                p->s.size -= nunits;
                p += p->s.size; // Move pointer to the allocated block.
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

/* =========================
   RTOS-Like Framework
   =========================
   Uses our custom allocator to allocate task stacks.
*/

#define MAX_TASKS 3
#define STACK_SIZE 256  // Number of uint32_t elements per task stack

typedef void (*TaskFunction_t)(void);

typedef struct {
    uint32_t *stackPointer;     // Pointer to the top of the allocated stack
    TaskFunction_t taskFunction; // Task entry function
} TCB_t;

TCB_t tasks[MAX_TASKS];
volatile int currentTask = 0;

// Initialize a task by allocating its stack using our custom allocator.
void initTask(int taskID, TaskFunction_t taskFunction) {
    uint32_t *stack = (uint32_t *)my_malloc(STACK_SIZE * sizeof(uint32_t));
    if (stack == NULL) {
        printf("Error allocating stack for task %d\n", taskID);
        exit(1);
    }
    // For a downward-growing stack, point to the top (end) of the allocated memory.
    tasks[taskID].stackPointer = stack + STACK_SIZE;
    tasks[taskID].taskFunction = taskFunction;
}

// Simple round-robin scheduler: cycles through the tasks.
void scheduler(void) {
    currentTask = (currentTask + 1) % MAX_TASKS;
}

// Simulated SysTick handler.
void SysTick_Handler(void) {
    scheduler();
}

// Start the simulated RTOS.
void startRTOS(void) {
    while (1) {
        tasks[currentTask].taskFunction();
        scheduler();
    }
}

// Example task functions.
void Task1(void) {
    printf("Task 1 is running\n");
}

void Task2(void) {
    printf("Task 2 is running\n");
}

void Task3(void) {
    printf("Task 3 is running\n");
}

int main(void) {
    // Initialize tasks with their respective functions.
    initTask(0, Task1);
    initTask(1, Task2);
    initTask(2, Task3);
    
    // Start the simulated RTOS.
    startRTOS();
    
    return 0;
}
