
.text
.align 2
.global	switchframe_switch
.type switchframe_switch, %function
switchframe_switch:
    sub sp, sp, #80

    str x19, [sp, #0]
    str x20, [sp, #8]
    str x21, [sp, #16]
    str x22, [sp, #24]
    str x23, [sp, #32]
    str x24, [sp, #40]
    str x25, [sp, #48]
    str x26, [sp, #56]
    str x27, [sp, #64]
    str x28, [sp, #72]
    str x30, [sp, #80]

    str x0, [sp, #0]

    ldr x1, [sp, #0]

    ldr x19, [sp, #0]
    ldr x20, [sp, #8]
    ldr x21, [sp, #16]
    ldr x22, [sp, #24]
    ldr x23, [sp, #32]
    ldr x24, [sp, #40]
    ldr x25, [sp, #48]
    ldr x26, [sp, #56]
    ldr x27, [sp, #64]
    ldr x28, [sp, #72]
    ldr x30, [sp, #80]

    add sp, sp, #80

    ret


.text
.align 2
.global	asm_adder
.type asm_adder, %function
asm_adder:
    add x2, x1, x0
    mov x0, x2
    br x30
