/*
 * startup.s - ARM bare-metal boot code for RPi2 (ARMv7-A, Cortex-A7)
 *
 * Entry point loaded at 0x8000 by the RPi GPU bootloader.
 * Sets up exception vectors, mode stacks, clears BSS, and jumps to C.
 */

.section .init, "ax"
.global _start

_start:
    ldr pc, =_reset_handler
    ldr pc, =_undefined_handler
    ldr pc, =_svc_handler
    ldr pc, =_prefetch_abort_handler
    ldr pc, =_data_abort_handler
    nop
    ldr pc, =_irq_handler
    ldr pc, =_fiq_handler

_reset_handler:
    /* Disable interrupts */
    cpsid if

    /* Park non-primary cores (RPi2/3/4 have 4 cores) */
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #3
    cmp r0, #0
    bne _park_core

    /* Set VBAR to our vector table */
    ldr r0, =_start
    mcr p15, 0, r0, c12, c0, 0

    /* Set up IRQ mode stack */
    cps #0x12
    ldr sp, =_irq_stack_top

    /* Set up Abort mode stack */
    cps #0x17
    ldr sp, =_abort_stack_top

    /* Set up Undefined mode stack */
    cps #0x1B
    ldr sp, =_undef_stack_top

    /* Set up SVC mode stack (kernel runs here) */
    cps #0x13
    ldr sp, =_svc_stack_top

    /* Clear BSS */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
_bss_loop:
    cmp r0, r1
    strlt r2, [r0], #4
    blt _bss_loop

    /* Jump to C kernel entry */
    bl kernel_main

    /* If kernel_main returns, hang */
_hang:
    wfi
    b _hang

_park_core:
    wfi
    b _park_core

/* Default exception handlers (weak, can be overridden) */
.weak _undefined_handler
_undefined_handler:
    b _undefined_handler

.weak _svc_handler
_svc_handler:
    b _svc_handler

.weak _prefetch_abort_handler
_prefetch_abort_handler:
    b _prefetch_abort_handler

.weak _data_abort_handler
_data_abort_handler:
    b _data_abort_handler

.weak _fiq_handler
_fiq_handler:
    b _fiq_handler
