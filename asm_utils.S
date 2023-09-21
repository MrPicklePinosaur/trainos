.text
.align 2
.global	priviledge_level
.type priviledge_level, %function
priviledge_level:
    mrs x0, CurrentEL
    and x0, x0, #0xC
    asr x0, x0, #2
    br x30

.text
.align 2
.global	exception_return_loc
.type exception_return_loc, %function
exception_return_loc:
    mrs x1, ELR_EL1
    ubfx x0, x1, #2, #2
    br x30

.global	vbar_value
.type vbar_value, %function
vbar_value:
    mrs x0, VBAR_EL1
    br x30

.global	asm_esr_el1
.type asm_esr_el1, %function
asm_esr_el1:
    mrs x0, ESR_EL1
    br x30

.global	asm_elr_el1
.type asm_elr_el1, %function
asm_elr_el1:
    mrs x0, ELR_EL1
    br x30

.global	asm_sp_el0
.type asm_sp_el0, %function
asm_sp_el0:
    mrs x0, SP_EL0
    br x30
