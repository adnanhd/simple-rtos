#include "uart.h"
#include "platform.h"

/* PL011 UART register offsets */
#define UART0_BASE      (PERIPHERAL_BASE + 0x201000)

#define UART_DR         (UART0_BASE + 0x00)
#define UART_FR         (UART0_BASE + 0x18)
#define UART_IBRD       (UART0_BASE + 0x24)
#define UART_FBRD       (UART0_BASE + 0x28)
#define UART_LCRH       (UART0_BASE + 0x2C)
#define UART_CR         (UART0_BASE + 0x30)
#define UART_IMSC       (UART0_BASE + 0x38)
#define UART_ICR        (UART0_BASE + 0x44)

/* Flag register bits */
#define UART_FR_TXFF    (1 << 5)
#define UART_FR_RXFE    (1 << 4)

void uart_init(void) {
    /* Disable UART */
    mmio_write(UART_CR, 0);

    /* Clear pending interrupts */
    mmio_write(UART_ICR, 0x7FF);

    /*
     * Set baud rate: 115200 at 3MHz UART clock (QEMU default)
     * Divider = 3000000 / (16 * 115200) = 1.627
     * IBRD = 1, FBRD = round(0.627 * 64) = 40
     */
    mmio_write(UART_IBRD, 1);
    mmio_write(UART_FBRD, 40);

    /* 8 data bits, FIFO enabled */
    mmio_write(UART_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    /* Disable all interrupts */
    mmio_write(UART_IMSC, 0);

    /* Enable UART, TX and RX */
    mmio_write(UART_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(char c) {
    while (mmio_read(UART_FR) & UART_FR_TXFF)
        ;
    mmio_write(UART_DR, (uint32_t)c);
}

char uart_getc(void) {
    while (mmio_read(UART_FR) & UART_FR_RXFE)
        ;
    return (char)(mmio_read(UART_DR) & 0xFF);
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}
