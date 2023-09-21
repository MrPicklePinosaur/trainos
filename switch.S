
.text
.align 2
.global	asm_enter_usermode
.type asm_enter_usermode, %function
asm_enter_usermode:

    ldr x19, [x0, #0]
    ldr x20, [x0, #8]
    ldr x21, [x0, #16]
    ldr x22, [x0, #24]
    ldr x23, [x0, #32]
    ldr x24, [x0, #40]
    ldr x25, [x0, #48]
    ldr x26, [x0, #56]
    ldr x27, [x0, #64]
    ldr x28, [x0, #72]
    ldr x30, [x0, #80]

    add x0, x0, #80

    msr ELR_EL1, x1

    msr SP_EL0, x0

    msr SPSR_EL1, xzr

    eret

.text
.align 2
.global	asm_adder
.type asm_adder, %function
asm_adder:
    add x2, x1, x0
    mov x0, x2
    br x30
