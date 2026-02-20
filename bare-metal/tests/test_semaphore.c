#include "unity.h"
#include "semaphore.h"
#include "scheduler.h"

/* Host stubs for irq_disable/irq_restore */
uint32_t irq_disable(void) { return 0; }
void irq_restore(uint32_t flags) { (void)flags; }

/* Track whether task_yield was called */
static int yield_called;

void task_yield(void) {
    yield_called++;
}

extern TCB_t *scheduler_get_task_pool(void);

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
    scheduler_init();
    yield_called = 0;
}

void tearDown(void) {
}

void test_init(void) {
    Semaphore_t sem;
    semaphore_init(&sem, 5);
    TEST_ASSERT_EQUAL_INT(5, sem.count);
    TEST_ASSERT_NULL(sem.waiting);
}

void test_signal_increments(void) {
    Semaphore_t sem;
    semaphore_init(&sem, 0);
    setup_current_task(1);

    semaphore_signal(&sem);
    TEST_ASSERT_EQUAL_INT(1, sem.count);
}

void test_wait_decrements(void) {
    Semaphore_t sem;
    semaphore_init(&sem, 2);
    setup_current_task(1);

    semaphore_wait(&sem);
    TEST_ASSERT_EQUAL_INT(1, sem.count);
    TEST_ASSERT_EQUAL_INT(0, yield_called);
}

void test_wait_blocks_when_zero(void) {
    Semaphore_t sem;
    semaphore_init(&sem, 0);
    TCB_t *task = setup_current_task(1);

    semaphore_wait(&sem);
    TEST_ASSERT_EQUAL_INT(-1, sem.count);
    TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, task->state);
    TEST_ASSERT_EQUAL_INT(1, yield_called);
    TEST_ASSERT_EQUAL_PTR(task, sem.waiting);
}

void test_signal_unblocks_waiter(void) {
    Semaphore_t sem;
    semaphore_init(&sem, 0);
    TCB_t *task = setup_current_task(1);

    /* Block the task */
    semaphore_wait(&sem);
    TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, task->state);

    /* Signal should unblock it */
    semaphore_signal(&sem);
    TEST_ASSERT_EQUAL(TASK_STATE_READY, task->state);
    TEST_ASSERT_NULL(sem.waiting);
}

void test_counting_semaphore(void) {
    Semaphore_t sem;
    semaphore_init(&sem, 3);
    setup_current_task(1);

    /* Three waits should succeed without blocking */
    semaphore_wait(&sem);
    semaphore_wait(&sem);
    semaphore_wait(&sem);
    TEST_ASSERT_EQUAL_INT(0, sem.count);
    TEST_ASSERT_EQUAL_INT(0, yield_called);

    /* Fourth wait should block */
    semaphore_wait(&sem);
    TEST_ASSERT_EQUAL_INT(-1, sem.count);
    TEST_ASSERT_EQUAL_INT(1, yield_called);
}

void test_multiple_waiters(void) {
    Semaphore_t sem;
    semaphore_init(&sem, 0);

    /* Block two tasks on the semaphore */
    TCB_t *t1 = setup_current_task(1);
    semaphore_wait(&sem);
    TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, t1->state);

    TCB_t *t2 = setup_current_task(1);
    semaphore_wait(&sem);
    TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, t2->state);

    /* Signal should unblock one at a time (LIFO order) */
    semaphore_signal(&sem);
    TEST_ASSERT_EQUAL(TASK_STATE_READY, t2->state);
    TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, t1->state);

    semaphore_signal(&sem);
    TEST_ASSERT_EQUAL(TASK_STATE_READY, t1->state);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init);
    RUN_TEST(test_signal_increments);
    RUN_TEST(test_wait_decrements);
    RUN_TEST(test_wait_blocks_when_zero);
    RUN_TEST(test_signal_unblocks_waiter);
    RUN_TEST(test_counting_semaphore);
    RUN_TEST(test_multiple_waiters);
    return UNITY_END();
}
