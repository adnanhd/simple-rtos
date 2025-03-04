#ifndef MQ_H
#define MQ_H

#include <stddef.h>

typedef void (*MessageCallback_t)(void *message, void *context);

typedef struct CallbackNode {
    MessageCallback_t callback;
    void *context;
    struct CallbackNode *next;
} CallbackNode;

typedef struct {
    CallbackNode *subscribers;
} MQ_t;

void mq_init(MQ_t *queue);
int mq_subscribe(MQ_t *queue, MessageCallback_t callback, void *context);
int mq_unsubscribe(MQ_t *queue, MessageCallback_t callback, void *context);
void mq_publish(MQ_t *queue, void *message);

#endif // MQ_H
