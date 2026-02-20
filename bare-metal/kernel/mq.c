#include "mq.h"
#include "mem.h"

void mq_init(MQ_t *queue) {
    queue->subscribers = NULL;
}

int mq_subscribe(MQ_t *queue, MessageCallback_t callback, void *context) {
    if (!callback)
        return -1;
    CallbackNode *node = (CallbackNode *)my_malloc(sizeof(CallbackNode));
    if (!node)
        return -1;
    node->callback = callback;
    node->context = context;
    node->next = queue->subscribers;
    queue->subscribers = node;
    return 0;
}

int mq_unsubscribe(MQ_t *queue, MessageCallback_t callback, void *context) {
    CallbackNode **prev = &queue->subscribers;
    while (*prev) {
        CallbackNode *curr = *prev;
        if (curr->callback == callback && curr->context == context) {
            *prev = curr->next;
            my_free(curr);
            return 0;
        }
        prev = &curr->next;
    }
    return -1;
}

void mq_publish(MQ_t *queue, void *message) {
    CallbackNode *curr = queue->subscribers;
    while (curr) {
        curr->callback(message, curr->context);
        curr = curr->next;
    }
}
