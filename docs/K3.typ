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
  title: "CS452 K2",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "October 5, 2023",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: `To be decided`

= Kernel Features

== AwaitEvent

AwaitEvent is implemented with no changes to its signature:
```c
int AwaitEvent(int eventType)
```

When a task calls AwaitEvent(eventid), it blocks until the event occurs.
Currently, the only event is the clock tick that happens every 10ms.

When an event occurs, the kernel will immediately unblock all tasks waiting for that event.
It will then consult the scheduler to determine the next task to run (using the standard metric of round-robining all unblocked tasks with the highest priority).

== Clock Server

The clock server implements the three methods as described on the assignment page, with no changes to their signatures:

```c
int Time(int tid)
int Delay(int tid, int ticks)
int DelayUntil(int tid, int ticks)
```

== Idle Task

The kernel measures how much time the idle task spends running.
The idle task itself stays in low power mode.

== Heap Allocator

Our heap allocator finally reclaims memory!
This means that our tasks can finally run indefinitely.

= Kernel Implementation

== Clock Server

The notifier task constantly calls AwaitEvent, waiting for the next clock tick.
After each tick, it sends a message to the clock server.
Our clock server tracks time by counting the number of these ticks.

When a task calls Delay() or DelayUntil(), the task blocks because of a Send() call.
The clock server then stores that task's Tid and the tick it should be unblocked at in a linked list.
Every tick, we loop through all tasks in this list, and reply to any that need to be unblocked on that tick.

The linked list might be inefficient, but that can be optimized at a later point if necessary.

Note that `Delay(ticks)` is simply implemented as `DelayUntil(current_tick + ticks)`.

== Idle Task

The system clock is used to measure the time spent running, not the clock server.
This is due to inaccuracies when using ticks to measure time.

Whenever we context switch into the idle task, the kernel stores the current time.
Then, when we context switch out of the idle task, we use that stored time to calculate the time we spent in the idle task.
This is done in the kernel rather than the idle task because it is easier to implement.
The idle task itself simply loops the WFI (Wait For Interrupt) instruction indefinitely, entering low power mode until an interrupt occurs.

Every second, we print the percentage of time that the idle task has been running.

= K3 Output

Based on the delays for each task, the outputs are in the following order.
Note that actual ticks printed might be one off from the ticks listed below; this is an expected inaccuracy caused by the way the tasks delay.

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
