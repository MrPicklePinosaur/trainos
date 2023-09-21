.text
.align 2

.global vector_table_init
.type vector_table_init, %function
vector_table_init:
    ldr x0, =vector_table_el1_base
    msr VBAR_EL1, x0
    br x30

.global MyTid
.type MyTid, %function
MyTid:
    
    svc 0x1

    br x30

.global Yield
.type Yield, %function
Yield:
    
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

    svc 0x3

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
    nop

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

.global return_from_svc
.type return_from_svc, %function
return_from_svc:
    eret
