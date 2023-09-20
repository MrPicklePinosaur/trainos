.text
.align 2
.global	priviledge_level
.type priviledge_level, %function
priviledge_level:
    mrs x1, CurrentEL
    ubfx x0, x1, #2, #2
    br x30

.text
.align 2
.global	exception_return_loc
.type exception_return_loc, %function
exception_return_loc:
    mrs x1, ELR_EL1
    ubfx x0, x1, #2, #2
    br x30
