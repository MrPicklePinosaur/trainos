.text
.align 2
.global	asm_enter_kernelmode
.type asm_enter_kernelmode, %function
asm_enter_kernelmode:
    
    # Grab current task switchframe and save necessary registers
    # WARNING: this procedure clobbers x1

    # frame to call tasktable functions
    sub sp, sp, #32
    str x0, [sp, #0]
    str x1, [sp, #8]
    str x2, [sp, #16]
    str x3, [sp, #24]
    str x30, [sp, #32]

    # x0 = tasktable_current_task()
    bl tasktable_current_task 

    # x0 = tasktable_get_task(x0)
    bl tasktable_get_task

    # x0 = task->sf
    ldr x0, [x0, #0]

    # save x0 (by grabbing it from top of stack)
    ldr x1, [sp, #0]
    str x1, [x0, #0]

    # notice that we don't load x0
    ldr x1, [sp, #8]
    ldr x2, [sp, #16]
    ldr x3, [sp, #24]
    ldr x30, [sp, #32]
    add sp, sp, #32

    str x1, [x0, #8]
    stp x2, x3, [x0, #16]
    stp x4, x5, [x0, #32]
    stp x6, x7, [x0, #48]
    stp x8, x9, [x0, #64]
    stp x10, x11, [x0, #80]
    stp x12, x13, [x0, #96]
    stp x14, x15, [x0, #112]
    stp x16, x17, [x0, #128]
    stp x18, x19, [x0, #144]
    stp x20, x21, [x0, #160]
    stp x22, x23, [x0, #176]
    stp x24, x25, [x0, #192]
    stp x26, x27, [x0, #208]
    stp x28, x30, [x0, #224]
    
    # save stack ptr
    mrs x1, SP_EL0
    str x1, [x0, #240]

    # load return value
    mrs x1, ELR_EL1
    str x1, [x0, #248]

    # load state reg
    mrs x1, SPSR_EL1
    str x1, [x0, #256]

    b handle_svc



asm_enter_kernelmode_from_interrupt:
    # Copied from asm_enter_kernelmode
    # Only difference is that the last line branches to handle_interrupt

    # Grab current task switchframe and save necessary registers
    # WARNING: this procedure clobbers x1

    # frame to call tasktable functions
    sub sp, sp, #32
    str x0, [sp, #0]
    str x1, [sp, #8]
    str x2, [sp, #16]
    str x3, [sp, #24]
    str x30, [sp, #32]

    # x0 = tasktable_current_task()
    bl tasktable_current_task

    # x0 = tasktable_get_task(x0)
    bl tasktable_get_task

    # x0 = task->sf
    ldr x0, [x0, #0]

    # save x0 (by grabbing it from top of stack)
    ldr x1, [sp, #0]
    str x1, [x0, #0]

    # notice that we don't load x0
    ldr x1, [sp, #8]
    ldr x2, [sp, #16]
    ldr x3, [sp, #24]
    ldr x30, [sp, #32]
    add sp, sp, #32

    str x1, [x0, #8]
    stp x2, x3, [x0, #16]
    stp x4, x5, [x0, #32]
    stp x6, x7, [x0, #48]
    stp x8, x9, [x0, #64]
    stp x10, x11, [x0, #80]
    stp x12, x13, [x0, #96]
    stp x14, x15, [x0, #112]
    stp x16, x17, [x0, #128]
    stp x18, x19, [x0, #144]
    stp x20, x21, [x0, #160]
    stp x22, x23, [x0, #176]
    stp x24, x25, [x0, #192]
    stp x26, x27, [x0, #208]
    stp x28, x30, [x0, #224]

    # save stack ptr
    mrs x1, SP_EL0
    str x1, [x0, #240]

    # load return value
    mrs x1, ELR_EL1
    str x1, [x0, #248]

    # load state reg
    mrs x1, SPSR_EL1
    str x1, [x0, #256]

    b handle_interrupt



.global	asm_enter_usermode
.type asm_enter_usermode, %function
asm_enter_usermode:

    ldp x2, x3, [x0, #16]
    ldp x4, x5, [x0, #32]
    ldp x6, x7, [x0, #48]
    ldp x8, x9, [x0, #64]
    ldp x10, x11, [x0, #80]
    ldp x12, x13, [x0, #96]
    ldp x14, x15, [x0, #112]
    ldp x16, x17, [x0, #128]
    ldp x18, x19, [x0, #144]
    ldp x20, x21, [x0, #160]
    ldp x22, x23, [x0, #176]
    ldp x24, x25, [x0, #192]
    ldp x26, x27, [x0, #208]
    ldp x28, x30, [x0, #224]

    # load stack ptr
    ldr x1, [x0, #240]
    msr SP_EL0, x1

    # load return value
    ldr x1, [x0, #248]
    msr ELR_EL1, x1

    # load state reg
    ldr x1, [x0, #256]
    msr SPSR_EL1, x1

    ldp x0, x1, [x0, #0]

    eret



.global vector_table_init
.type vector_table_init, %function
vector_table_init:
    ldr x0, =vector_table_el1_base
    msr VBAR_EL1, x0
    br x30


// vector table from https://developer.arm.com/documentation/100933/0100/AArch64-exception-vector-table
.balign 0x800
vector_table_el1_base:
curr_el_sp0_sync:        // The exception handler for a synchronous
                         // exception from the current EL using SP0.
    nop
.balign 0x80
curr_el_sp0_irq:         // The exception handler for an IRQ exception
                         // from the current EL using SP0.
    nop
.balign 0x80
curr_el_sp0_fiq:         // The exception handler for an FIQ exception
                         // from the current EL using SP0.
    nop
.balign 0x80
curr_el_sp0_serror:      // The exception handler for a System Error
                         // exception from the current EL using SP0.
    nop
.balign 0x80
curr_el_spx_sync:        // The exception handler for a synchrous
                         // exception from the current EL using the
                         // current SP.
    nop
.balign 0x80
curr_el_spx_irq:         // The exception handler for an IRQ exception from
                         // the current EL using the current SP.
    nop

.balign 0x80
curr_el_spx_fiq:         // The exception handler for an FIQ from
                         // the current EL using the current SP.
    nop

.balign 0x80
curr_el_spx_serror:      // The exception handler for a System Error
                         // exception from the current EL using the
                         // current SP.
    nop

.balign 0x80
lower_el_aarch64_sync:   // The exception handler for a synchronous
                         // exception from a lower EL (AArch64).
    b asm_enter_kernelmode

.balign 0x80
lower_el_aarch64_irq:    // The exception handler for an IRQ from a lower EL
                         // (AArch64).
    b asm_enter_kernelmode_from_interrupt

.balign 0x80
lower_el_aarch64_fiq:    // The exception handler for an FIQ from a lower EL
                         // (AArch64).
    nop

.balign 0x80
lower_el_aarch64_serror: // The exception handler for a System Error
                         // exception from a lower EL(AArch64).
    nop

.balign 0x80
lower_el_aarch32_sync:   // The exception handler for a synchronous
                         // exception from a lower EL(AArch32).
    nop
.balign 0x80
lower_el_aarch32_irq:    // The exception handler for an IRQ exception
                         // from a lower EL (AArch32).
    nop
.balign 0x80
lower_el_aarch32_fiq:    // The exception handler for an FIQ exception from
                         // a lower EL (AArch32).
    nop
.balign 0x80
lower_el_aarch32_serror: // The exception handler for a System Error
                         // exception from a lower EL(AArch32).
    nop
