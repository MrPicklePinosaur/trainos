.text
.align 2
.global	priviledge_level
.type priviledge_level, %function
priviledge_level:
    mrs x1, CurrentEL
    ubfx x0, x1, #2, #2

