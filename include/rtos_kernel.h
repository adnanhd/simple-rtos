#ifndef RTOS_KERNEL_H
#define RTOS_KERNEL_H

#include "rtos.h"

// Internal kernel prototypes (used within the RTOS implementation)
// Note: Advanced IPC functions (e.g. RTOS_WaitMessage, RTOS_GetMessage) have been
// removed from the core API. Use the ipc module instead for blocking message operations.

// Function to simulate a tick (internal use only)
void internal_RTOS_Tick(void);

#endif // RTOS_KERNEL_H
