/*
 * vectors.s - IRQ exception handler stub
 *
 * On IRQ entry, ARM is in IRQ mode with banked lr_irq and spsr_irq.
 * We save the interrupted context onto the SVC stack, call the C handler,
 * then restore and return.
 */

.text
.global _irq_handler

_irq_handler:
    /* Adjust lr for IRQ return (ARM quirk: lr_irq = PC+4 of interrupted instr) */
    sub lr, lr, #4

    /* Save lr and spsr to SVC mode stack */
    srsdb sp!, #0x13

    /* Switch to SVC mode (interrupts still disabled) */
    cps #0x13

    /* Save all registers */
    push {r0-r12, lr}

    /* Call C IRQ dispatcher */
    bl irq_dispatch

    /* Restore registers */
    pop {r0-r12, lr}

    /* Return from exception: pops PC and CPSR saved by srsdb */
    rfeia sp!
