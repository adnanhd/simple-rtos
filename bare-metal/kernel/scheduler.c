#include "scheduler.h"

typedef struct {
    TCB_t *head;
    TCB_t *tail;
} ReadyQueue_t;

static ReadyQueue_t ready_queues[MAX_PRIORITIES];
static TCB_t task_pool[MAX_TASKS];
static uint32_t next_task_id;

TCB_t *current_tcb;

void scheduler_init(void) {
    for (int i = 0; i < MAX_PRIORITIES; i++) {
        ready_queues[i].head = NULL;
        ready_queues[i].tail = NULL;
    }
    for (int i = 0; i < MAX_TASKS; i++) {
        task_pool[i].state = TASK_STATE_DEAD;
        task_pool[i].next = NULL;
    }
    current_tcb = NULL;
    next_task_id = 0;
}

static void enqueue_ready(TCB_t *tcb) {
    uint8_t p = tcb->priority;
    tcb->next = NULL;
    if (ready_queues[p].tail != NULL) {
        ready_queues[p].tail->next = tcb;
    } else {
        ready_queues[p].head = tcb;
    }
    ready_queues[p].tail = tcb;
}

static TCB_t *dequeue_ready(uint8_t priority) {
    ReadyQueue_t *q = &ready_queues[priority];
    if (q->head == NULL)
        return NULL;
    TCB_t *tcb = q->head;
    q->head = tcb->next;
    if (q->head == NULL)
        q->tail = NULL;
    tcb->next = NULL;
    return tcb;
}

TCB_t *scheduler_alloc_task(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].state == TASK_STATE_DEAD) {
            task_pool[i].task_id = next_task_id++;
            return &task_pool[i];
        }
    }
    return NULL;
}

void scheduler_add_task(TCB_t *tcb) {
    tcb->state = TASK_STATE_READY;
    tcb->time_slice = DEFAULT_TIME_SLICE;
    enqueue_ready(tcb);
}

void scheduler_remove_task(TCB_t *tcb) {
    uint8_t p = tcb->priority;
    ReadyQueue_t *q = &ready_queues[p];

    if (q->head == NULL)
        return;

    if (q->head == tcb) {
        q->head = tcb->next;
        if (q->head == NULL)
            q->tail = NULL;
        tcb->next = NULL;
        return;
    }

    TCB_t *prev = q->head;
    while (prev->next && prev->next != tcb)
        prev = prev->next;

    if (prev->next == tcb) {
        prev->next = tcb->next;
        if (q->tail == tcb)
            q->tail = prev;
        tcb->next = NULL;
    }
}

TCB_t *scheduler_select_next(void) {
    for (int p = 0; p < MAX_PRIORITIES; p++) {
        TCB_t *next = dequeue_ready((uint8_t)p);
        if (next != NULL) {
            next->state = TASK_STATE_RUNNING;
            next->time_slice = DEFAULT_TIME_SLICE;
            current_tcb = next;
            return next;
        }
    }
    /* No ready tasks — return current (should be idle task) */
    return current_tcb;
}

void scheduler_tick(void) {
    /* Decrement sleeping tasks and wake them if ready */
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].state == TASK_STATE_SLEEPING) {
            if (task_pool[i].delay_ticks > 0)
                task_pool[i].delay_ticks--;
            if (task_pool[i].delay_ticks == 0) {
                task_pool[i].state = TASK_STATE_READY;
                enqueue_ready(&task_pool[i]);
            }
        }
    }

    /* Decrement current task's time slice */
    if (current_tcb && current_tcb->state == TASK_STATE_RUNNING) {
        if (current_tcb->time_slice > 0)
            current_tcb->time_slice--;
        if (current_tcb->time_slice == 0) {
            current_tcb->state = TASK_STATE_READY;
            enqueue_ready(current_tcb);
        }
    }
}

/* Expose task pool for testing and kernel use */
TCB_t *scheduler_get_task_pool(void) {
    return task_pool;
}
