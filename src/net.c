#include "net.h"
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// We'll use an IPC_t queue for incoming network packets.
static IPC_t rx_q;
#define RX_QUEUE_CAPACITY 10

int net_init(void) {
    if (ipc_init(&rx_q, RX_QUEUE_CAPACITY) != 0)
        return -1;
    return 0;
}

int net_send(const void *packet, size_t length) {
    printf("net_send: Sending packet: %.*s\n", (int)length, (const char *)packet);
    return 0;
}

int net_receive(void *buffer, size_t buffer_length, size_t *received_length) {
    void *msg;
    int ret = ipc_receive(&rx_q, &msg);
    if (ret != 0)
        return ret;
    size_t msg_len = strlen((char *)msg);
    if (msg_len >= buffer_length)
        msg_len = buffer_length - 1;
    memcpy(buffer, msg, msg_len);
    ((char *)buffer)[msg_len] = '\0';
    if (received_length)
        *received_length = msg_len;
    return 0;
}

int net_sim(const char *packet) {
    char *copy = strdup(packet);
    if (!copy)
        return -1;
    return ipc_send(&rx_q, copy);
}
