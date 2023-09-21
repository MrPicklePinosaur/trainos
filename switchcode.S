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
    
    ; sub sp, sp, #232

    ; stp x0, x1, [sp, #0]
    ; stp x2, x3, [sp, #16]
    ; stp x4, x5, [sp, #32]
    ; stp x6, x7, [sp, #48]
    ; stp x8, x9, [sp, #64]
    ; stp x10, x11, [sp, #80]
    ; stp x12, x13, [sp, #96]
    ; stp x14, x15, [sp, #112]
    ; stp x16, x17, [sp, #128]
    ; stp x18, x19, [sp, #144]
    ; stp x20, x21, [sp, #160]
    ; stp x22, x23, [sp, #176]
    ; stp x24, x25, [sp, #192]
    ; stp x26, x27, [sp, #208]
    ; stp x28, x30, [sp, #224]
    
    svc 0x1

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
    b handle_svc

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

; vector_table:
;     mrs x0, esr_el1
;     str x0, [sp, #0]
;     mrs x0, elr_el1
;     str x0, [sp, #8]
;     mrs x0, spsr_el1
;     str x0, [sp, #16]
;     mrs x0, sp_el0
;     str x0, [sp, #24]
; 
;     sub sp, sp, #24
; 
;     b handle_svc

; .align 2
; .global pop_stack
; pop_stack:
;     ldr x0, [sp, #0]
;     add sp, sp, #8
;     br x30

.global return_from_svc
.type return_from_svc, %function
return_from_svc:
    eret
