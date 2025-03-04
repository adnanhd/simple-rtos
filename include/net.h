#ifndef NET_H
#define NET_H

#include <stddef.h>

// Initialize the network stack.
int net_init(void);
// Send a packet.
int net_send(const void *packet, size_t length);
// Blocking receive: waits until a packet is available.
int net_receive(void *buffer, size_t buffer_length, size_t *received_length);
// Function to simulate an incoming packet.
int net_sim(const char *packet);

#endif // NET_H
