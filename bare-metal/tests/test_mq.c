#include "unity.h"
#include "mq.h"
#include "mem.h"

static MQ_t queue;
static int callback_count;
static void *last_message;
static void *last_context;

static void test_callback(void *message, void *context) {
    callback_count++;
    last_message = message;
    last_context = context;
}

static int second_callback_count;

static void second_callback(void *message, void *context) {
    (void)message;
    (void)context;
    second_callback_count++;
}

void setUp(void) {
    init_allocator();
    mq_init(&queue);
    callback_count = 0;
    second_callback_count = 0;
    last_message = NULL;
    last_context = NULL;
}

void tearDown(void) {
}

void test_init_subscribers_null(void) {
    TEST_ASSERT_NULL(queue.subscribers);
}

void test_subscribe_and_publish(void) {
    int ctx = 42;
    int ret = mq_subscribe(&queue, test_callback, &ctx);
    TEST_ASSERT_EQUAL_INT(0, ret);

    int msg = 99;
    mq_publish(&queue, &msg);

    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL_PTR(&msg, last_message);
    TEST_ASSERT_EQUAL_PTR(&ctx, last_context);
}

void test_multiple_subscribers(void) {
    mq_subscribe(&queue, test_callback, NULL);
    mq_subscribe(&queue, second_callback, NULL);

    int msg = 7;
    mq_publish(&queue, &msg);

    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL_INT(1, second_callback_count);
}

void test_unsubscribe(void) {
    int ctx = 1;
    mq_subscribe(&queue, test_callback, &ctx);
    mq_subscribe(&queue, second_callback, NULL);

    int ret = mq_unsubscribe(&queue, test_callback, &ctx);
    TEST_ASSERT_EQUAL_INT(0, ret);

    int msg = 5;
    mq_publish(&queue, &msg);

    TEST_ASSERT_EQUAL_INT(0, callback_count);
    TEST_ASSERT_EQUAL_INT(1, second_callback_count);
}

void test_unsubscribe_nonexistent(void) {
    int ret = mq_unsubscribe(&queue, test_callback, NULL);
    TEST_ASSERT_EQUAL_INT(-1, ret);
}

void test_subscribe_null_callback(void) {
    int ret = mq_subscribe(&queue, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(-1, ret);
}

void test_publish_no_subscribers(void) {
    int msg = 10;
    mq_publish(&queue, &msg); /* Should not crash */
    TEST_ASSERT_EQUAL_INT(0, callback_count);
}

void test_publish_multiple_times(void) {
    mq_subscribe(&queue, test_callback, NULL);

    int msg1 = 1, msg2 = 2, msg3 = 3;
    mq_publish(&queue, &msg1);
    mq_publish(&queue, &msg2);
    mq_publish(&queue, &msg3);

    TEST_ASSERT_EQUAL_INT(3, callback_count);
    TEST_ASSERT_EQUAL_PTR(&msg3, last_message);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_subscribers_null);
    RUN_TEST(test_subscribe_and_publish);
    RUN_TEST(test_multiple_subscribers);
    RUN_TEST(test_unsubscribe);
    RUN_TEST(test_unsubscribe_nonexistent);
    RUN_TEST(test_subscribe_null_callback);
    RUN_TEST(test_publish_no_subscribers);
    RUN_TEST(test_publish_multiple_times);
    return UNITY_END();
}
