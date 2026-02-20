#include "unity.h"
#include "mem.h"

void setUp(void) {
    init_allocator();
}

void tearDown(void) {
}

void test_malloc_returns_non_null(void) {
    void *ptr = my_malloc(100);
    TEST_ASSERT_NOT_NULL(ptr);
}

void test_malloc_different_pointers(void) {
    void *a = my_malloc(64);
    void *b = my_malloc(64);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_EQUAL(a, b);
}

void test_free_and_reuse(void) {
    void *a = my_malloc(128);
    TEST_ASSERT_NOT_NULL(a);
    my_free(a);
    void *b = my_malloc(128);
    TEST_ASSERT_NOT_NULL(b);
}

void test_malloc_zero_returns_non_null(void) {
    void *ptr = my_malloc(0);
    TEST_ASSERT_NOT_NULL(ptr);
}

void test_write_and_read_back(void) {
    uint8_t *ptr = (uint8_t *)my_malloc(256);
    TEST_ASSERT_NOT_NULL(ptr);
    for (int i = 0; i < 256; i++)
        ptr[i] = (uint8_t)i;
    for (int i = 0; i < 256; i++)
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, ptr[i]);
}

void test_realloc_preserves_data(void) {
    uint8_t *ptr = (uint8_t *)my_malloc(64);
    TEST_ASSERT_NOT_NULL(ptr);
    for (int i = 0; i < 64; i++)
        ptr[i] = (uint8_t)(i + 1);

    uint8_t *new_ptr = (uint8_t *)my_realloc(ptr, 128);
    TEST_ASSERT_NOT_NULL(new_ptr);
    for (int i = 0; i < 64; i++)
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(i + 1), new_ptr[i]);
}

void test_realloc_null_acts_as_malloc(void) {
    void *ptr = my_realloc(NULL, 100);
    TEST_ASSERT_NOT_NULL(ptr);
}

void test_realloc_zero_frees(void) {
    void *ptr = my_malloc(100);
    TEST_ASSERT_NOT_NULL(ptr);
    void *result = my_realloc(ptr, 0);
    TEST_ASSERT_NULL(result);
}

void test_free_null_is_safe(void) {
    my_free(NULL);  /* Should not crash */
}

void test_exhaust_heap(void) {
    /* Allocate in large chunks until the heap is exhausted */
    void *ptrs[128];
    int count = 0;
    for (int i = 0; i < 128; i++) {
        ptrs[i] = my_malloc(16384);  /* 16KB chunks */
        if (ptrs[i] == NULL)
            break;
        count++;
    }
    TEST_ASSERT_GREATER_THAN(0, count);
    /* Eventually should fail */
    void *fail = my_malloc(UINT32_MAX / 2);
    TEST_ASSERT_NULL(fail);

    /* Clean up */
    for (int i = 0; i < count; i++)
        my_free(ptrs[i]);
}

void test_many_small_allocs(void) {
    void *ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = my_malloc(8);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
    }
    for (int i = 0; i < 100; i++)
        my_free(ptrs[i]);
    /* Should be able to allocate again after freeing */
    void *ptr = my_malloc(800);
    TEST_ASSERT_NOT_NULL(ptr);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_malloc_returns_non_null);
    RUN_TEST(test_malloc_different_pointers);
    RUN_TEST(test_free_and_reuse);
    RUN_TEST(test_malloc_zero_returns_non_null);
    RUN_TEST(test_write_and_read_back);
    RUN_TEST(test_realloc_preserves_data);
    RUN_TEST(test_realloc_null_acts_as_malloc);
    RUN_TEST(test_realloc_zero_frees);
    RUN_TEST(test_free_null_is_safe);
    RUN_TEST(test_exhaust_heap);
    RUN_TEST(test_many_small_allocs);
    return UNITY_END();
}
