/*
 * kernel.ld - Linker script for RPi2 bare-metal kernel
 * Kernel loaded at 0x8000 by the GPU bootloader
 */

ENTRY(_start)

SECTIONS
{
    . = 0x8000;

    /* Vector table must come first */
    .init : {
        KEEP(*(.init))
    }

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    . = ALIGN(4);
    __bss_start = .;
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    }
    . = ALIGN(4);
    __bss_end = .;

    /* Heap for memory allocator */
    . = ALIGN(8);
    __heap_start = .;
    . = . + 1M;
    __heap_end = .;

    /* Mode stacks (grow downward) */
    . = ALIGN(8);
    _svc_stack_bottom = .;
    . = . + 8K;
    _svc_stack_top = .;

    _irq_stack_bottom = .;
    . = . + 4K;
    _irq_stack_top = .;

    _abort_stack_bottom = .;
    . = . + 4K;
    _abort_stack_top = .;

    _undef_stack_bottom = .;
    . = . + 4K;
    _undef_stack_top = .;

    _end = .;

    /DISCARD/ : {
        *(.comment)
        *(.ARM.attributes)
    }
}
