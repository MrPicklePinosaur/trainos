.text
.align 2
.global	asm_enter_kernelmode
.type asm_enter_kernelmode, %function
asm_enter_kernelmode:
    
    # Grab current task switchframe and save necessary registers
    # WARNING: this procedure clobbers x1

    # push x0 and x1
    sub sp, sp, #16
    str x0, [sp, #0]
    str x1, [sp, #8]

    # x0 = tasktable_current_task()
    bl tasktable_current_task 

    # x0 = tasktable_get_task(x0)
    bl tasktable_get_task

    # x0 = task->sf
    ldr x0, [x0, #0]

    # save x1
    str x1, [x0, #8]

    # save x0 (by grabbing it from top of stack)
    ldr x1, [sp, #0]
    str x1, [x0, #0]

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

    # pop x0 and x1
    ldr x0, [sp, #0]
    ldr x1, [sp, #8]
    add sp, sp, #16

    b handle_svc



.text
.align 2
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
    ldr x0, [x0, #240]
    msr SP_EL0, x0

    # load return value
    ldr x0, [x0, #248]
    msr ELR_EL1, x0

    # load state reg
    ldr x0, [x0, #256]
    msr SPSR_EL1, x0

    ldp x0, x1, [x0, #0]

    eret
