#include "ipc.h"
#include "mem.h"
#include "scheduler.h"
#include "irq.h"

/* Weak symbol — overridden by real implementation on bare-metal */
__attribute__((weak)) void task_yield(void) { }

static void add_waiting_consumer(IPC_t *q, TCB_t *task) {
    WaitingNode *node = (WaitingNode *)my_malloc(sizeof(WaitingNode));
    node->task = task;
    node->next = q->waitingConsumers;
    q->waitingConsumers = node;
}

static TCB_t *pop_waiting_consumer(IPC_t *q) {
    if (!q->waitingConsumers)
        return NULL;
    WaitingNode *node = q->waitingConsumers;
    q->waitingConsumers = node->next;
    TCB_t *task = node->task;
    my_free(node);
    return task;
}

static void add_waiting_producer(IPC_t *q, TCB_t *task) {
    WaitingNode *node = (WaitingNode *)my_malloc(sizeof(WaitingNode));
    node->task = task;
    node->next = q->waitingProducers;
    q->waitingProducers = node;
}

static TCB_t *pop_waiting_producer(IPC_t *q) {
    if (!q->waitingProducers)
        return NULL;
    WaitingNode *node = q->waitingProducers;
    q->waitingProducers = node->next;
    TCB_t *task = node->task;
    my_free(node);
    return task;
}

int ipc_init(IPC_t *q, size_t capacity) {
    q->buffer = (void **)my_malloc(capacity * sizeof(void *));
    if (!q->buffer)
        return -1;
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->waitingConsumers = NULL;
    q->waitingProducers = NULL;
    return 0;
}

void ipc_destroy(IPC_t *q) {
    if (q->buffer)
        my_free(q->buffer);
    while (q->waitingConsumers) {
        WaitingNode *node = q->waitingConsumers;
        q->waitingConsumers = node->next;
        my_free(node);
    }
    while (q->waitingProducers) {
        WaitingNode *node = q->waitingProducers;
        q->waitingProducers = node->next;
        my_free(node);
    }
}

int ipc_send(IPC_t *q, void *message) {
    uint32_t flags = irq_disable();
    if (q->count == q->capacity) {
        add_waiting_producer(q, current_tcb);
        current_tcb->state = TASK_STATE_BLOCKED;
        irq_restore(flags);
        task_yield();
        flags = irq_disable();
    }
    q->buffer[q->tail] = message;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    if (q->waitingConsumers) {
        TCB_t *consumer = pop_waiting_consumer(q);
        if (consumer) {
            consumer->state = TASK_STATE_READY;
            scheduler_add_task(consumer);
        }
    }
    irq_restore(flags);
    return 0;
}

int ipc_receive(IPC_t *q, void **message) {
    uint32_t flags = irq_disable();
    if (q->count == 0) {
        add_waiting_consumer(q, current_tcb);
        current_tcb->state = TASK_STATE_BLOCKED;
        irq_restore(flags);
        task_yield();
        flags = irq_disable();
    }
    *message = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    if (q->waitingProducers) {
        TCB_t *producer = pop_waiting_producer(q);
        if (producer) {
            producer->state = TASK_STATE_READY;
            scheduler_add_task(producer);
        }
    }
    irq_restore(flags);
    return 0;
}
