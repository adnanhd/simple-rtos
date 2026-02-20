#ifndef RTOS_IPC_H
#define RTOS_IPC_H

#include "types.h"
#include "kernel.h"

typedef struct WaitingNode {
    TCB_t *task;
    struct WaitingNode *next;
} WaitingNode;

typedef struct {
    void **buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    WaitingNode *waitingConsumers;
    WaitingNode *waitingProducers;
} IPC_t;

int ipc_init(IPC_t *q, size_t capacity);
void ipc_destroy(IPC_t *q);
int ipc_send(IPC_t *q, void *message);
int ipc_receive(IPC_t *q, void **message);

#endif /* RTOS_IPC_H */
