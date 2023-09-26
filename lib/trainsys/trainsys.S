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

.global Send
.type Send, %function
Send:
    svc 0x5
    br x30

.global Receive
.type Receive, %function
Receive:
    svc 0x6
    br x30

.global Reply
.type Reply, %function
Reply:
    svc 0x7
    br x30

