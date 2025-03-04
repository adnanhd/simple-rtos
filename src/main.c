#include <stdio.h>
#include "rtos.h"
#include "rtos_kernel.h"
#include "callback_mq.h"

// Declare a global callback message queue.
CallbackMessageQueue msgQueue;

void SenderTask(void) {
    int counter = 0;
    char msgBuffer[50];
    while (1) {
        // Prepare a message.
        sprintf(msgBuffer, "Message %d from SenderTask", counter++);
        printf("SenderTask: Publishing message...\n");
        callback_mq_publish(&msgQueue, msgBuffer);
        RTOS_Delay(4000000);  // Delay for 4 ticks before sending next message.
    }
}

void ReceiverTask(void) {
    while (1) {
        printf("ReceiverTask: Waiting for a message...\n");
        RTOS_WaitMessage(&msgQueue);  // Block until a message is published.
        char *received = (char *)RTOS_GetMessage();
        printf("ReceiverTask: Received: %s\n", received);
        RTOS_Delay(2000000);  // Optionally delay before waiting again.
    }
}

void OtherTask(void) {
    int count = 0;
    while (1) {
        printf("OtherTask: Doing independent work (%d)\n", count++);
        RTOS_Delay(300000);
    }
}

int main(void) {
    // Initialize the callback message queue.
    callback_mq_init(&msgQueue);
    
    // Initialize the RTOS.
    RTOS_Init();
    
    // Create user tasks. (Note: idleTask is automatically created in RTOS_Init.)
    RTOS_CreateTask(SenderTask);
    RTOS_CreateTask(ReceiverTask);
    RTOS_CreateTask(OtherTask);
    
    // Start the RTOS scheduler.
    RTOS_Start();
    return 0;
}
