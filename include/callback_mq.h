#ifndef CALLBACK_MQ_H
#define CALLBACK_MQ_H

#include <stddef.h>

// Callback function type: receives the message and a user-supplied context.
typedef void (*MessageCallback_t)(void *message, void *context);

// Node representing one subscriber.
typedef struct CallbackNode {
    MessageCallback_t callback;
    void *context;
    struct CallbackNode *next;
} CallbackNode;

// The callback-based message queue structure.
typedef struct {
    CallbackNode *subscribers;
} CallbackMessageQueue;

// Initialize the callback message queue.
void callback_mq_init(CallbackMessageQueue *queue);

// Subscribe a callback to the message queue.
int callback_mq_subscribe(CallbackMessageQueue *queue, MessageCallback_t callback, void *context);

// Unsubscribe a callback from the message queue.
int callback_mq_unsubscribe(CallbackMessageQueue *queue, MessageCallback_t callback, void *context);

// Publish a message to the queue, triggering all callbacks.
void callback_mq_publish(CallbackMessageQueue *queue, void *message);

#endif // CALLBACK_MQ_H
