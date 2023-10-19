#let project(title: "", authors: (), date: none, body) = {
  set document(author: authors, title: title)
  set page(numbering: "1", number-align: center)
  set text(font: "New Computer Modern", lang: "en")
  show math.equation: set text(weight: 400)

  align(center)[
    #block(text(weight: 700, 1.75em, title))
    #v(1em, weak: true)
    #date
  ]

  pad(
    top: 0.5em,
    bottom: 0.5em,
    x: 2em,
    grid(
      columns: (1fr,) * calc.min(3, authors.len()),
      gutter: 1em,
      ..authors.map(author => align(center, strong(author))),
    ),
  )

  set par(justify: true)

  body
}

#show: project.with(
  title: "CS452 K3",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "October 19, 2023",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: f2ba77c0df906c2b83fce98ce57b04e0dc19de2a

= Kernel Features

== AwaitEvent

AwaitEvent is implemented with no changes to its signature:
```c
int AwaitEvent(int eventType)
```

When a task calls `AwaitEvent(eventid)`, it blocks until the event occurs.
Currently, the only event is the clock tick that happens every 10ms.

When an event occurs, the kernel will immediately unblock all tasks waiting for that event.
It will then consult the scheduler to determine the next task to run (using the round-robin algorithm to select an unblocked task with the highest priority).

== Clock Server

The clock server implements the following three methods as described on the assignment page, with no changes to their signatures:

```c
int Time(int tid)
int Delay(int tid, int ticks)
int DelayUntil(int tid, int ticks)
```

== Idle Task

The kernel properly measures how much time the idle task spends running.
The idle task itself stays in low power mode by repeatedly running the `WFI` instruction in a loop.

== Memory Optimizations

Since our memory allocator still lacks the ability to reclaim memory, and the interrupt handler
allocates memory each time, we were running out of memory very
quickly. Rather than implementing memory reclamation, we optimized some parts
of the codebase to no longer require memory allocations.

A proper malloc/free implementation is in the works but it is not currently in
use for K3 due to lack of testing.

== Virtual Machine

We have the ability to run the kernel inside Qemu.
However, Qemu is only able to simulate the Raspberry Pi 3B, so an additional compile flag `-DQemu` is used to conditionally compile the kernel for Raspberry Pi 3B compatibility.
It runs seamlessly for the most part, although interrupts are currently untested.
The simulator is useful when we are not in the lab, writing code that doesn't involve most of the devices.
It is also possible to use GDB to debug.

= Kernel Implementation

== AwaitEvent

AwaitEvent is implemented by introducing a new task state called `TASKSTATE_AWAIT_EVENT_WAIT`.
The scheduler skips over tasks with this state when choosing the next task to run.

When an event is triggered, the scheduler iterates through all tasks, unblocking any that are awaiting that event.

== Clock Server

The notifier task constantly calls AwaitEvent, waiting for the next clock tick event.
When it receives that event, it sends a message to the clock server.
Our clock server tracks time by counting the number of these messages.

When a task calls `Delay()` or `DelayUntil()`, the task `Send()`s a message to the clock server.
When the clock server `Receive()`s that message, it does not immediately `Reply()`.
Instead, it stores that task's Tid and the tick it should be unblocked at in a linked list.
(For `Delay()` calls, the tick to unblock at is calculated as `current_tick + ticks`).
Every tick, we loop through all tasks in this linked list, and `Reply()` to any that need to be unblocked on that tick.

The linked list might be inefficient, but that can be optimized at a later point if necessary.
For now, due to the low amount of tasks that delay at once, this is fine.

== Idle Task

Our first attempt at measuring idle task time used the clock server.
We would repeatedly call `DelayUntil(current_tick+1)`, incrementing a counter after every time.
This would measure the number of ticks that occurred in the idle task, when no task other task was running.
However, this led to a reported 100% idle time, since the total time of all other operations was always under a tick.

Our new implementation uses the kernel and system timer to measure time more accurately.
Whenever we context switch into the idle task, the kernel stores the current time.
Then, when we context switch out of the idle task, we use that stored time to calculate the time it spent running.
This led to a more accurate measurement of about 90% idle time.

Note that the kernel is what tracks idle time, not the idle task.
The idle task simply loops the WFI (Wait For Interrupt) instruction indefinitely, entering low power mode until an interrupt occurs.

Another user task is used to print out the idle time.
It repeatedly delays for one second, queries the kernel for the idle time, and prints it.

= K3 Output

Based on the delays for each task, the outputs should be in the following order.

#table(
  columns: (auto, auto, auto, auto, auto, auto),
  [Task], [Ticks 1], [Ticks 2], [Ticks 3], [Ticks 4], [Num Delays],
  [8], [10], [], [], [], [1],
  [8], [20], [], [], [], [2],
  [9], [], [23], [], [], [1],
  [8], [30], [], [], [], [3],
  [10], [], [], [33], [], [1],
  [8], [40], [], [], [], [4],
  [9], [], [46], [], [], [2],
  [8], [50], [], [], [], [5],
  [8], [60], [], [], [], [6],
  [10], [], [], [66], [], [2],
  [9], [], [69], [], [], [3],
  [8], [70], [], [], [], [7],
  [11], [], [], [], [71], [1],
  [8], [80], [], [], [], [8],
  [8], [90], [], [], [], [9],
  [9], [], [92], [], [], [4],
  [10], [], [], [99], [], [3],
  [8], [100], [], [], [], [10],
  [8], [110], [], [], [], [11],
  [9], [], [115], [], [], [5],
  [8], [120], [], [], [], [12],
  [8], [130], [], [], [], [13],
  [10], [], [], [132], [], [4],
  [9], [], [138], [], [], [6],
  [8], [140], [], [], [], [14],
  [11], [], [], [], [142], [2],
  [8], [150], [], [], [], [15],
  [8], [160], [], [], [], [16],
  [9], [], [161], [], [], [7],
  [10], [], [], [165], [], [5],
  [8], [170], [], [], [], [17],
  [8], [180], [], [], [], [18],
  [9], [], [184], [], [], [8],
  [8], [190], [], [], [], [19],
  [10], [], [], [198], [], [6],
  [8], [200], [], [], [], [20],
  [9], [], [207], [], [], [9],
  [11], [], [], [], [213], [3]
)

Below is the console output from one run of our program.
Notice that sometimes the outputted tick will be a tick or two greater than the expected amount.
This is because `Delay` blocks the caller until _at least_ the specified number of ticks, and due to interrupts or other tasks taking time to run, this can push back the tracked ticks by a bit.
```
Tid: 8, delay interval: 10, completed delays: 1, tick: 10
Tid: 8, delay interval: 10, completed delays: 2, tick: 20
Tid: 9, delay interval: 23, completed delays: 1, tick: 23
Tid: 8, delay interval: 10, completed delays: 3, tick: 30
Tid: 10, delay interval: 33, completed delays: 1, tick: 34
Tid: 8, delay interval: 10, completed delays: 4, tick: 40
Tid: 9, delay interval: 23, completed delays: 2, tick: 46
Tid: 8, delay interval: 10, completed delays: 5, tick: 50
Tid: 8, delay interval: 10, completed delays: 6, tick: 60
Tid: 10, delay interval: 33, completed delays: 2, tick: 67
Tid: 9, delay interval: 23, completed delays: 3, tick: 69
Tid: 8, delay interval: 10, completed delays: 7, tick: 70
Tid: 11, delay interval: 71, completed delays: 1, tick: 72
Tid: 8, delay interval: 10, completed delays: 8, tick: 80
Tid: 8, delay interval: 10, completed delays: 9, tick: 90
Tid: 9, delay interval: 23, completed delays: 4, tick: 92
Tid: 8, delay interval: 10, completed delays: 10, tick: 100
Tid: 10, delay interval: 33, completed delays: 3, tick: 100
Tid: 8, delay interval: 10, completed delays: 11, tick: 110
Tid: 9, delay interval: 23, completed delays: 5, tick: 115
Tid: 8, delay interval: 10, completed delays: 12, tick: 120
Tid: 8, delay interval: 10, completed delays: 13, tick: 130
Tid: 10, delay interval: 33, completed delays: 4, tick: 134
Tid: 9, delay interval: 23, completed delays: 6, tick: 138
Tid: 8, delay interval: 10, completed delays: 14, tick: 140
Tid: 11, delay interval: 71, completed delays: 2, tick: 143
Tid: 8, delay interval: 10, completed delays: 15, tick: 150
Tid: 8, delay interval: 10, completed delays: 16, tick: 160
Tid: 9, delay interval: 23, completed delays: 7, tick: 161
Tid: 10, delay interval: 33, completed delays: 5, tick: 167
Tid: 8, delay interval: 10, completed delays: 17, tick: 170
Tid: 8, delay interval: 10, completed delays: 18, tick: 180
Tid: 9, delay interval: 23, completed delays: 8, tick: 184
Tid: 8, delay interval: 10, completed delays: 19, tick: 190
Tid: 8, delay interval: 10, completed delays: 20, tick: 200
Tid: 10, delay interval: 33, completed delays: 6, tick: 200
Tid: 9, delay interval: 23, completed delays: 9, tick: 207
Tid: 11, delay interval: 71, completed delays: 3, tick: 214
```
