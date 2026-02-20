#include "kernel.h"
#include "scheduler.h"
#include "semaphore.h"
#include "ipc.h"
#include "mem.h"
#include "uart.h"
#include "timer.h"
#include "kprintf.h"
#include "irq.h"

extern int task_create(TaskFunction_t func, uint8_t priority);
extern void task_sleep(uint32_t ticks);
extern void task_yield(void);
extern void irq_enable(void);

static Semaphore_t shared_sem;
static IPC_t msg_queue;

static void task_high(void) {
    int count = 0;
    while (1) {
        kprintf("[HIGH] tick %d\n", count++);
        task_sleep(500);
    }
}

static void task_medium(void) {
    int count = 0;
    while (1) {
        semaphore_wait(&shared_sem);
        kprintf("[MED]  holding semaphore, iteration %d\n", count++);
        task_sleep(100);
        semaphore_signal(&shared_sem);
        task_sleep(300);
    }
}

static void task_producer(void) {
    int count = 0;
    while (1) {
        kprintf("[PROD] sending %d\n", count);
        ipc_send(&msg_queue, (void *)(uintptr_t)count);
        count++;
        task_sleep(400);
    }
}

static void task_consumer(void) {
    while (1) {
        void *msg;
        ipc_receive(&msg_queue, &msg);
        kprintf("[CONS] received %d\n", (int)(uintptr_t)msg);
    }
}

static void idle_task(void) {
    while (1) {
        __asm__ volatile("wfi");
    }
}

void kernel_main(void) {
    uart_init();
    kprintf_init(uart_putc);
    init_allocator();
    scheduler_init();

    kprintf("\n=== RTOS Bare-Metal Microkernel ===\n");
    kprintf("Initializing...\n");

    semaphore_init(&shared_sem, 1);
    ipc_init(&msg_queue, 8);

    task_create(task_high,     0);  /* Priority 0 (highest) */
    task_create(task_medium,   2);
    task_create(task_producer,  4);
    task_create(task_consumer,  4);
    task_create(idle_task,      7);  /* Lowest priority */

    kprintf("Starting scheduler...\n");

    /* Start the first task */
    TCB_t *first = scheduler_select_next();
    if (first) {
        /* Enable timer (1ms tick) and interrupts */
        timer_init(1000);
        irq_enable();

        /* Load the first task's context (never returns) */
        __asm__ volatile(
            "mov sp, %0\n"
            "pop {r0-r12, lr}\n"
            "pop {r2}\n"
            "msr cpsr_c, r2\n"
            "bx lr\n"
            :
            : "r"(first->sp)
            : "memory"
        );
    }

    /* Should never reach here */
    kprintf("ERROR: No tasks to run!\n");
    while (1)
        __asm__ volatile("wfi");
}
