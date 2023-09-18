
# Steps during context switch

- save callee-saved registers x19-x30 on the stack
- save 'from' stack pointer
- restore 'to' stack pointer
- pop callee-saved register x19-x30
