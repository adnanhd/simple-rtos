#ifndef RTOS_KERNEL_H
#define RTOS_KERNEL_H

#include "types.h"

#define MAX_PRIORITIES  8
#define MAX_TASKS       16
#define TASK_STACK_SIZE 4096
#define DEFAULT_TIME_SLICE 10

typedef void (*TaskFunction_t)(void);

typedef enum {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_SLEEPING,
    TASK_STATE_DEAD
} TaskState_t;

typedef struct TCB {
    uint32_t    *sp;
    struct TCB  *next;
    uint8_t     priority;
    TaskState_t state;
    uint32_t    delay_ticks;
    uint32_t    time_slice;
    void        *message;
    uint32_t    task_id;
    uint32_t    *stack_base;
} TCB_t;

#endif /* RTOS_KERNEL_H */
