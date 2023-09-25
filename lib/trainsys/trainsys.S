.text
.align 2

.global Create
.type Create, %function
Create:
    svc 0x0
    br x30

.global MyTid
.type MyTid, %function
MyTid:
    svc 0x1
    br x30

.global MyParentTid
.type MyParentTid, %function
MyParentTid:
    svc 0x2
    br x30

.global Yield
.type Yield, %function
Yield:
    svc 0x3
    br x30

.global Exit
.type Exit, %function
Exit:
    svc 0x4
    br x30

