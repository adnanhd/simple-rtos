#include "unity.h"
#include "scheduler.h"

/* Access task pool for test setup */
extern TCB_t *scheduler_get_task_pool(void);

static TCB_t *make_task(uint8_t priority) {
    TCB_t *pool = scheduler_get_task_pool();
    for (int i = 0; i < MAX_TASKS; i++) {
        if (pool[i].state == TASK_STATE_DEAD) {
            pool[i].priority = priority;
            pool[i].delay_ticks = 0;
            pool[i].time_slice = DEFAULT_TIME_SLICE;
            pool[i].next = NULL;
            pool[i].message = NULL;
            pool[i].sp = NULL;
            pool[i].stack_base = NULL;
            scheduler_add_task(&pool[i]);
            return &pool[i];
        }
    }
    return NULL;
}

void setUp(void) {
    scheduler_init();
}

void tearDown(void) {
}

void test_select_highest_priority(void) {
    TCB_t *low = make_task(3);
    TCB_t *high = make_task(0);

    TCB_t *selected = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(high, selected);
    TEST_ASSERT_EQUAL(TASK_STATE_RUNNING, high->state);

    /* Next select should return the low-priority task */
    TCB_t *selected2 = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(low, selected2);
}

void test_round_robin_same_priority(void) {
    TCB_t *a = make_task(2);
    TCB_t *b = make_task(2);
    TCB_t *c = make_task(2);

    TCB_t *s1 = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(a, s1);

    /* Put a back in ready queue to simulate yield */
    a->state = TASK_STATE_READY;
    scheduler_add_task(a);

    TCB_t *s2 = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(b, s2);

    b->state = TASK_STATE_READY;
    scheduler_add_task(b);

    TCB_t *s3 = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(c, s3);

    c->state = TASK_STATE_READY;
    scheduler_add_task(c);

    /* Should cycle back to a */
    TCB_t *s4 = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(a, s4);
}

void test_tick_decrements_sleeping(void) {
    TCB_t *pool = scheduler_get_task_pool();
    TCB_t *t = &pool[0];
    t->priority = 1;
    t->state = TASK_STATE_SLEEPING;
    t->delay_ticks = 5;

    scheduler_tick();
    TEST_ASSERT_EQUAL_UINT32(4, t->delay_ticks);
    TEST_ASSERT_EQUAL(TASK_STATE_SLEEPING, t->state);
}

void test_sleeping_task_wakes(void) {
    TCB_t *pool = scheduler_get_task_pool();
    TCB_t *t = &pool[0];
    t->priority = 1;
    t->state = TASK_STATE_SLEEPING;
    t->delay_ticks = 1;

    scheduler_tick();
    TEST_ASSERT_EQUAL_UINT32(0, t->delay_ticks);
    TEST_ASSERT_EQUAL(TASK_STATE_READY, t->state);

    /* Should now be selectable */
    TCB_t *selected = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(t, selected);
}

void test_time_slice_preemption(void) {
    TCB_t *a = make_task(1);
    TCB_t *b = make_task(1);

    TCB_t *selected = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(a, selected);
    TEST_ASSERT_EQUAL(TASK_STATE_RUNNING, a->state);

    /* Tick down the time slice */
    for (int i = 0; i < DEFAULT_TIME_SLICE; i++)
        scheduler_tick();

    /* a should be moved back to ready */
    TEST_ASSERT_EQUAL(TASK_STATE_READY, a->state);

    /* Next select should pick b */
    TCB_t *next = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(b, next);
}

void test_blocked_task_skipped(void) {
    TCB_t *a = make_task(1);
    TCB_t *b = make_task(1);

    /* Remove a from ready and block it */
    scheduler_remove_task(a);
    a->state = TASK_STATE_BLOCKED;

    TCB_t *selected = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(b, selected);
}

void test_empty_returns_current(void) {
    /* Set up an idle task as current */
    TCB_t *idle = make_task(MAX_PRIORITIES - 1);
    current_tcb = idle;
    idle->state = TASK_STATE_RUNNING;

    /* Remove idle from ready queue since it's "running" */
    scheduler_remove_task(idle);

    /* No ready tasks — should return current */
    TCB_t *selected = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(idle, selected);
}

void test_multiple_priorities_interleave(void) {
    TCB_t *high1 = make_task(0);
    TCB_t *high2 = make_task(0);
    TCB_t *low = make_task(5);

    /* High priority tasks should be selected first */
    TCB_t *s1 = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(high1, s1);

    TCB_t *s2 = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(high2, s2);

    /* Only after both high tasks are consumed, low gets picked */
    TCB_t *s3 = scheduler_select_next();
    TEST_ASSERT_EQUAL_PTR(low, s3);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_select_highest_priority);
    RUN_TEST(test_round_robin_same_priority);
    RUN_TEST(test_tick_decrements_sleeping);
    RUN_TEST(test_sleeping_task_wakes);
    RUN_TEST(test_time_slice_preemption);
    RUN_TEST(test_blocked_task_skipped);
    RUN_TEST(test_empty_returns_current);
    RUN_TEST(test_multiple_priorities_interleave);
    return UNITY_END();
}
