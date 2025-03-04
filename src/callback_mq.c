#include "callback_mq.h"
#include <stdlib.h>

void callback_mq_init(CallbackMessageQueue *queue) {
    queue->subscribers = NULL;
}

int callback_mq_subscribe(CallbackMessageQueue *queue, MessageCallback_t callback, void *context) {
    if (!callback) return -1;
    CallbackNode *node = (CallbackNode *)malloc(sizeof(CallbackNode));
    if (!node) return -1;
    node->callback = callback;
    node->context = context;
    node->next = queue->subscribers;
    queue->subscribers = node;
    return 0;
}

int callback_mq_unsubscribe(CallbackMessageQueue *queue, MessageCallback_t callback, void *context) {
    CallbackNode **prev = &queue->subscribers;
    while (*prev) {
        CallbackNode *curr = *prev;
        if (curr->callback == callback && curr->context == context) {
            *prev = curr->next;
            free(curr);
            return 0;
        }
        prev = &curr->next;
    }
    return -1;
}

void callback_mq_publish(CallbackMessageQueue *queue, void *message) {
    CallbackNode *curr = queue->subscribers;
    while (curr) {
        curr->callback(message, curr->context);
        curr = curr->next;
    }
}
