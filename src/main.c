#include "rtos.h"
#include "net.h"
#include <stdio.h>

void NetSimTask(void) {
    int count = 10000;
    char msg[50];
    while (1) {
        sprintf(msg, "Packet %d", count++);
        printf("NetSimTask: Simulating packet: %s\n", msg);
        net_sim(msg);
        net_sim(msg);
        RTOS_Delay(5000000);
    }
}

void NetRecvTask1(void) {
    char buf[100];
    size_t rlen;
    while (1) {
        printf("NetRecvTask1: Waiting for packet...\n");
        net_receive(buf, sizeof(buf), &rlen);
        printf("NetRecvTas1: Received: %s\n", buf);
        RTOS_Delay(3);
    }
}

void NetRecvTask2(void) {
    char buf[100];
    size_t rlen;
    while (1) {
        printf("NetRecvTask2: Waiting for packet...\n");
        net_receive(buf, sizeof(buf), &rlen);
        printf("NetRecvTask2: Received: %s\n", buf);
        RTOS_Delay(3);
    }
}

int main(void) {
    if (net_init() != 0) {
        printf("Network initialization failed\n");
        return -1;
    }
    RTOS_Init();
    RTOS_CreateTask(NetSimTask);
    RTOS_CreateTask(NetRecvTask1);
    RTOS_CreateTask(NetRecvTask2);
    RTOS_Start();
    return 0;
}
