#include "kprintf.h"
#include <stdarg.h>

static kputc_fn out_putc;

void kprintf_init(kputc_fn putc) {
    out_putc = putc;
}

static void put_char(char c) {
    if (out_putc)
        out_putc(c);
}

static void put_string(const char *s) {
    while (*s)
        put_char(*s++);
}

static void put_unsigned(uint32_t val, int base, int uppercase) {
    char buf[32];
    int i = 0;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    if (val == 0) {
        put_char('0');
        return;
    }

    while (val > 0) {
        buf[i++] = digits[val % base];
        val /= base;
    }

    while (i > 0)
        put_char(buf[--i]);
}

static void put_signed(int32_t val) {
    if (val < 0) {
        put_char('-');
        /* Handle INT32_MIN safely */
        put_unsigned((uint32_t)(-(int64_t)val), 10, 0);
    } else {
        put_unsigned((uint32_t)val, 10, 0);
    }
}

void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            put_char(*fmt++);
            continue;
        }

        fmt++; /* skip '%' */

        switch (*fmt) {
        case 'd':
        case 'i':
            put_signed(va_arg(ap, int32_t));
            break;
        case 'u':
            put_unsigned(va_arg(ap, uint32_t), 10, 0);
            break;
        case 'x':
            put_unsigned(va_arg(ap, uint32_t), 16, 0);
            break;
        case 'X':
            put_unsigned(va_arg(ap, uint32_t), 16, 1);
            break;
        case 'p': {
            uintptr_t ptr = (uintptr_t)va_arg(ap, void *);
            put_string("0x");
            put_unsigned((uint32_t)ptr, 16, 0);
            break;
        }
        case 's': {
            const char *s = va_arg(ap, const char *);
            put_string(s ? s : "(null)");
            break;
        }
        case 'c':
            put_char((char)va_arg(ap, int));
            break;
        case '%':
            put_char('%');
            break;
        case '\0':
            goto done;
        default:
            put_char('%');
            put_char(*fmt);
            break;
        }
        fmt++;
    }

done:
    va_end(ap);
}
