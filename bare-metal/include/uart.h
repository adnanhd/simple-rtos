#ifndef RTOS_UART_H
#define RTOS_UART_H

void uart_init(void);
void uart_putc(char c);
char uart_getc(void);
void uart_puts(const char *s);

#endif /* RTOS_UART_H */
