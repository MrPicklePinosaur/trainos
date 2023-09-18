
; .text
; .global	switchframe_switch
; .type switchframe_switch, %function
; switchframe_switch:

.text
.align 2
.global	asm_adder
.type asm_adder, %function
asm_adder:
  add x2, x1, x0
  mov x0, x2
  br x30
