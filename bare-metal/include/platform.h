#ifndef RTOS_PLATFORM_H
#define RTOS_PLATFORM_H

#include "types.h"

/* Peripheral base address per platform */
#if defined(PLATFORM_RPI4)
    #define PERIPHERAL_BASE 0xFE000000
#elif defined(PLATFORM_RPI2) || defined(PLATFORM_RPI3)
    #define PERIPHERAL_BASE 0x3F000000
#else
    /* Default to RPi2/3 (also used by QEMU raspi2b) */
    #define PERIPHERAL_BASE 0x3F000000
#endif

/* MMIO helpers */
static inline void mmio_write(uint32_t addr, uint32_t value) {
    *(volatile uint32_t *)addr = value;
}

static inline uint32_t mmio_read(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

/* Memory barrier (required for RPi peripherals) */
static inline void dmb(void) {
    __asm__ volatile("dmb" ::: "memory");
}

#endif /* RTOS_PLATFORM_H */
