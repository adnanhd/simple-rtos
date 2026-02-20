# Makefile for bare-metal ARM microkernel (RPi2 / QEMU raspi2b)

CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
AS      = $(CROSS)as
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump

PLATFORM ?= RPI2

CFLAGS  = -mcpu=cortex-a7 -mfloat-abi=soft \
          -ffreestanding -nostdlib -nostartfiles \
          -Wall -Wextra -O2 -g \
          -DPLATFORM_$(PLATFORM) \
          -Iinclude

ASFLAGS = -mcpu=cortex-a7

LDFLAGS = -T kernel.ld -nostdlib

# Source files
ASM_SRCS = arch/startup.s arch/context_switch.s arch/vectors.s
C_SRCS   = kernel/kernel.c kernel/scheduler.c kernel/semaphore.c \
           kernel/ipc.c kernel/mq.c kernel/mem.c kernel/irq.c \
           kernel/kprintf.c \
           drivers/uart.c drivers/timer.c \
           app/main.c

OBJDIR   = build

ASM_OBJS = $(patsubst arch/%.s, $(OBJDIR)/arch/%.o, $(ASM_SRCS))
C_OBJS   = $(patsubst %.c, $(OBJDIR)/%.o, $(C_SRCS))
OBJS     = $(ASM_OBJS) $(C_OBJS)

TARGET   = kernel

.PHONY: all clean qemu qemu-debug

all: $(OBJDIR)/$(TARGET).elf $(OBJDIR)/$(TARGET).bin $(OBJDIR)/kernel7.img

$(OBJDIR)/$(TARGET).elf: $(OBJS) kernel.ld
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS) -lgcc
	$(OBJDUMP) -d $@ > $(OBJDIR)/$(TARGET).list

$(OBJDIR)/$(TARGET).bin: $(OBJDIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(OBJDIR)/kernel7.img: $(OBJDIR)/$(TARGET).bin
	cp $< $@

$(OBJDIR)/arch/%.o: arch/%.s
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

# QEMU launch (raspi2b emulates BCM2836 RPi2)
qemu: $(OBJDIR)/$(TARGET).elf
	qemu-system-arm -M raspi2b -kernel $< \
	    -serial stdio -display none -no-reboot

# QEMU with GDB server (connect with: arm-none-eabi-gdb build/kernel.elf -ex "target remote :1234")
qemu-debug: $(OBJDIR)/$(TARGET).elf
	qemu-system-arm -M raspi2b -kernel $< \
	    -serial stdio -display none -no-reboot \
	    -S -gdb tcp::1234
