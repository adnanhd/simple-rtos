#include "timer.h"
#include "platform.h"

/* BCM2835 ARM Timer registers */
#define ARM_TIMER_BASE  (PERIPHERAL_BASE + 0xB000)
#define ARM_TIMER_LOD   (ARM_TIMER_BASE + 0x400)
#define ARM_TIMER_VAL   (ARM_TIMER_BASE + 0x404)
#define ARM_TIMER_CTL   (ARM_TIMER_BASE + 0x408)
#define ARM_TIMER_CLI   (ARM_TIMER_BASE + 0x40C)
#define ARM_TIMER_RIS   (ARM_TIMER_BASE + 0x410)
#define ARM_TIMER_MIS   (ARM_TIMER_BASE + 0x414)
#define ARM_TIMER_RLD   (ARM_TIMER_BASE + 0x418)
#define ARM_TIMER_DIV   (ARM_TIMER_BASE + 0x41C)

/* BCM2835 interrupt controller */
#define IRQ_BASE            (PERIPHERAL_BASE + 0xB000)
#define IRQ_ENABLE_BASIC    (IRQ_BASE + 0x218)

void timer_init(uint32_t interval_us) {
    /*
     * APB clock on RPi2 ~ 250MHz
     * Pre-divider = 249 -> timer clock = 250MHz / 250 = 1MHz
     * Load value = interval_us (e.g. 1000 for 1ms tick)
     */
    mmio_write(ARM_TIMER_DIV, 249);
    mmio_write(ARM_TIMER_LOD, interval_us);
    mmio_write(ARM_TIMER_RLD, interval_us);
    mmio_write(ARM_TIMER_CLI, 0);

    /* Enable timer, enable timer IRQ, 23-bit counter */
    mmio_write(ARM_TIMER_CTL, (1 << 7) | (1 << 5) | (1 << 1));

    /* Enable ARM Timer IRQ in interrupt controller (bit 0 of basic enable) */
    mmio_write(IRQ_ENABLE_BASIC, (1 << 0));
}

void timer_irq_handler(void) {
    /* Acknowledge the timer interrupt */
    mmio_write(ARM_TIMER_CLI, 0);
}
