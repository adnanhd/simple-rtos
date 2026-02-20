#include "unity.h"
#include "ipc.h"
#include "mem.h"
#include "scheduler.h"

/* Host stubs */
uint32_t irq_disable(void) { return 0; }
void irq_restore(uint32_t flags) { (void)flags; }

static int yield_called;
void task_yield(void) { yield_called++; }

extern TCB_t *scheduler_get_task_pool(void);

static IPC_t queue;

static TCB_t *setup_current_task(uint8_t priority) {
    TCB_t *pool = scheduler_get_task_pool();
    for (int i = 0; i < MAX_TASKS; i++) {
        if (pool[i].state == TASK_STATE_DEAD) {
            pool[i].priority = priority;
            pool[i].state = TASK_STATE_RUNNING;
            pool[i].next = NULL;
            current_tcb = &pool[i];
            return &pool[i];
        }
    }
    return NULL;
}

void setUp(void) {
    init_allocator();
    scheduler_init();
    yield_called = 0;
    ipc_init(&queue, 4);
    setup_current_task(1);
}

void tearDown(void) {
    ipc_destroy(&queue);
}

void test_init_and_destroy(void) {
    IPC_t q;
    int ret = ipc_init(&q, 8);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_UINT(8, q.capacity);
    TEST_ASSERT_EQUAL_UINT(0, q.count);
    TEST_ASSERT_NULL(q.waitingConsumers);
    TEST_ASSERT_NULL(q.waitingProducers);
    ipc_destroy(&q);
}

void test_send_receive_single(void) {
    int msg = 42;
    ipc_send(&queue, &msg);
    TEST_ASSERT_EQUAL_UINT(1, queue.count);

    void *received;
    ipc_receive(&queue, &received);
    TEST_ASSERT_EQUAL_PTR(&msg, received);
    TEST_ASSERT_EQUAL_UINT(0, queue.count);
}

void test_send_receive_fifo_order(void) {
    int a = 1, b = 2, c = 3;
    ipc_send(&queue, &a);
    ipc_send(&queue, &b);
    ipc_send(&queue, &c);

    void *r;
    ipc_receive(&queue, &r);
    TEST_ASSERT_EQUAL_PTR(&a, r);
    ipc_receive(&queue, &r);
    TEST_ASSERT_EQUAL_PTR(&b, r);
    ipc_receive(&queue, &r);
    TEST_ASSERT_EQUAL_PTR(&c, r);
}

void test_fill_to_capacity(void) {
    int msgs[4] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++)
        ipc_send(&queue, &msgs[i]);
    TEST_ASSERT_EQUAL_UINT(4, queue.count);

    /* Verify all come back in order */
    for (int i = 0; i < 4; i++) {
        void *r;
        ipc_receive(&queue, &r);
        TEST_ASSERT_EQUAL_PTR(&msgs[i], r);
    }
}

void test_receive_empty_blocks(void) {
    TCB_t *task = setup_current_task(1);

    void *r;
    ipc_receive(&queue, &r);

    TEST_ASSERT_EQUAL_INT(1, yield_called);
    TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, task->state);
}

void test_send_full_blocks(void) {
    int msgs[4] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++)
        ipc_send(&queue, &msgs[i]);

    TCB_t *task = setup_current_task(1);
    int extra = 5;
    ipc_send(&queue, &extra);

    TEST_ASSERT_EQUAL_INT(1, yield_called);
    TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, task->state);
}

void test_send_unblocks_consumer(void) {
    /* Set up a consumer that blocks */
    TCB_t *consumer = setup_current_task(1);
    void *r;
    ipc_receive(&queue, &r);
    TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, consumer->state);

    /* Now send — should unblock the consumer */
    setup_current_task(1);
    int msg = 99;
    ipc_send(&queue, &msg);
    TEST_ASSERT_EQUAL(TASK_STATE_READY, consumer->state);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_and_destroy);
    RUN_TEST(test_send_receive_single);
    RUN_TEST(test_send_receive_fifo_order);
    RUN_TEST(test_fill_to_capacity);
    RUN_TEST(test_receive_empty_blocks);
    RUN_TEST(test_send_full_blocks);
    RUN_TEST(test_send_unblocks_consumer);
    return UNITY_END();
}
