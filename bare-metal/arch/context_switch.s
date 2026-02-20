/*
 * context_switch.s - ARM context switch routines
 *
 * Saves and restores: r0-r12, lr, cpsr (15 words = 60 bytes per frame)
 */

.text
.global context_switch
.global task_initial_entry

/*
 * context_switch(uint32_t **old_sp, uint32_t *new_sp)
 *   r0 = pointer to old task's saved SP (in TCB)
 *   r1 = new task's saved SP (from TCB)
 */
context_switch:
    /* Save current context */
    mrs r2, cpsr
    push {r2}
    push {r0-r12, lr}

    /* Store current SP into old task's TCB */
    str sp, [r0]

    /* Switch to new task's stack */
    mov sp, r1

    /* Restore new task's context */
    pop {r0-r12, lr}
    pop {r2}
    msr cpsr_c, r2
    bx lr

/*
 * task_initial_entry
 * First-time entry for a new task. The task function pointer is in r4.
 */
task_initial_entry:
    mov r0, r4
    blx r0
    /* If the task function returns, mark task as dead and yield */
    bl task_exit
    b .
