#include "unity.h"
#include "kprintf.h"
#include <string.h>

/* Capture output into a buffer */
static char output_buf[1024];
static int output_pos;

static void test_putc(char c) {
    if (output_pos < (int)sizeof(output_buf) - 1)
        output_buf[output_pos++] = c;
    output_buf[output_pos] = '\0';
}

void setUp(void) {
    output_pos = 0;
    output_buf[0] = '\0';
    kprintf_init(test_putc);
}

void tearDown(void) {
}

void test_plain_string(void) {
    kprintf("hello");
    TEST_ASSERT_EQUAL_STRING("hello", output_buf);
}

void test_decimal_positive(void) {
    kprintf("%d", 42);
    TEST_ASSERT_EQUAL_STRING("42", output_buf);
}

void test_decimal_negative(void) {
    kprintf("%d", -1);
    TEST_ASSERT_EQUAL_STRING("-1", output_buf);
}

void test_decimal_zero(void) {
    kprintf("%d", 0);
    TEST_ASSERT_EQUAL_STRING("0", output_buf);
}

void test_unsigned(void) {
    kprintf("%u", (uint32_t)4294967295U);
    TEST_ASSERT_EQUAL_STRING("4294967295", output_buf);
}

void test_hex_lowercase(void) {
    kprintf("%x", 0xff);
    TEST_ASSERT_EQUAL_STRING("ff", output_buf);
}

void test_hex_uppercase(void) {
    kprintf("%X", 0xff);
    TEST_ASSERT_EQUAL_STRING("FF", output_buf);
}

void test_hex_zero(void) {
    kprintf("%x", 0);
    TEST_ASSERT_EQUAL_STRING("0", output_buf);
}

void test_char(void) {
    kprintf("%c", 'A');
    TEST_ASSERT_EQUAL_STRING("A", output_buf);
}

void test_string_format(void) {
    kprintf("%s", "world");
    TEST_ASSERT_EQUAL_STRING("world", output_buf);
}

void test_null_string(void) {
    kprintf("%s", (char *)NULL);
    TEST_ASSERT_EQUAL_STRING("(null)", output_buf);
}

void test_percent_literal(void) {
    kprintf("100%%");
    TEST_ASSERT_EQUAL_STRING("100%", output_buf);
}

void test_pointer(void) {
    kprintf("%p", (void *)0x1234);
    TEST_ASSERT_EQUAL_STRING("0x1234", output_buf);
}

void test_mixed_format(void) {
    kprintf("task %d: %s = %x", 3, "val", 0xab);
    TEST_ASSERT_EQUAL_STRING("task 3: val = ab", output_buf);
}

void test_int_min(void) {
    kprintf("%d", (int32_t)(-2147483647 - 1));
    TEST_ASSERT_EQUAL_STRING("-2147483648", output_buf);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_plain_string);
    RUN_TEST(test_decimal_positive);
    RUN_TEST(test_decimal_negative);
    RUN_TEST(test_decimal_zero);
    RUN_TEST(test_unsigned);
    RUN_TEST(test_hex_lowercase);
    RUN_TEST(test_hex_uppercase);
    RUN_TEST(test_hex_zero);
    RUN_TEST(test_char);
    RUN_TEST(test_string_format);
    RUN_TEST(test_null_string);
    RUN_TEST(test_percent_literal);
    RUN_TEST(test_pointer);
    RUN_TEST(test_mixed_format);
    RUN_TEST(test_int_min);
    return UNITY_END();
}
