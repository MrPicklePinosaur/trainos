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
  title: "CS452 K4",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "October 26, 2023",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: `faba3a1eb3be8c57118b89406d6929045529ba18`

= Startup

The kernel begins by printing the TrainOS logo.
Just below the logo is a section labelled `TODAY'S ROLL`.
This is purely decorative and can be ignored.
#footnote[
  It's a gacha system which rolls a random schedule of UW courses, and gives each a rarity between 1-pip and 5-pip.
  It will reroll every time you boot up the kernel. See if you can get a 5-pip CS 452!
]

Below that section is the task list, prompted with the words `SELECT TASK TO RUN`.
Each task has a corresponding number, and you can input the number of a task to run it.

 - `K1` is the K1 task which creates 4 children tasks of varying priorities.
 - `K2` is the K2 rock paper scissors demo.
 - `K2perf` is the K2 Send/Receive/Reply performance measurement.
 - `K3` is the K3 clock server tests.
 - `K4` is the train controller.
 - `sendReceiveReplyTest` is a deprecated, broken test.
 - `graphics` tests our UI library.
 - `test` is our suite of unit tests.

For this assignment, you only really need to run `K4`.

== Marklin Train Controller

The Marklin control TUI from Assignment 0 was ported to `trainos`.
It uses tasks and message passing to lower the latency of data updates and UI re-renders.

If you run the `K4` task, you should be brought to the train controller.
The controller includes a prompt to enter commands, a history of entered commands, a list of recently triggered sensors, a table of switch states, and some diagnostic information.

The prompt allows you to use backspace to delete characters.
More advanced text manipulation is not supported.

The following commands are supported:

`tr <train number> <speed>`
Controls the speed of a given train.
`<train number>` is between 1 and 80, and speed is between 0 and 14.

`rv <train number>`
Reverses the direction of a given train.

`sw <switch number> <switch direction>`
Sets the state of a switch.
`<switch number>` is between 1 and 18, or between 153 and 156.
`<switch direction>` must be "S" (for straight) or "C" (for curved).
Note that these characters must be capitalized.

`light <train number> <on|off>`
Toggle the lights on the train on and off.

`go`
Turns on the Marklin.

`stop`
Turns off the Marklin.

`q`
Quits the program.

= Kernel Implementation

== Task Creation

Our kernel implements the following five syscalls for creating and handling tasks:

```c
int Create(int priority, void (*function)(), const char* name)
int MyTid()
int MyParentTid()
void Yield()
void Exit()
```

Note that `Create()` has been modified to take in a string called `name`.
This string is an identifier for the task, used for debugging.

The first task created has a TID of 1.
Task IDs are given out incrementally, so the second task has a TID of 2, the third a TID of 3, and so on.
We do not reuse TIDs.

No task can have a TID of 0.
A TID of 0 acts as an error value for functions that should return a TID but cannot.

Our kernel schedules tasks using a round robin algorithm to cycle through all unblocked tasks of the highest priority.

There is an idle task, so it is impossible for there to be no available tasks to run.

Theoretically, if we ignore the limited memory of the RPi, there is no limit to the amount of tasks we can create.

== Memory Layout

We allocate 2MB for the kernel stack, and each task gets a 1MB page to use as a stack.
Pages are reclaimed once their task exits.

We have implemented a heap, which lets us allocate unknown amounts of memory during runtime.
This is useful since working with stack and static memory all the time can be cumbersome.

All tasks share a single heap.
We use a freelist to find unused memory.

== Standard Library

Our memory allocator has allowed us to implement some standard library data structures, such as strings, hash maps, and linked lists.
These have been very useful in implementating our kernel.

== Debug Logging

Our debug logging allows us to mask categories of debug statements as needed.
For example, if we need to debug the interrupt handler, we can specifically allow only interrupt handler debug statements through.

== Task Table

Each task has its metadata stored in a `Task` struct.
This struct is heap-allocated.

Pointers to these structs are stored in a hash map called the task table.
The hash map allows us to efficiently retrieve task data.
The hash of a task is simply its TID modulo 128.
Theoretically, this will start operating slowly if many tasks are run at once, but for our use cases, this is more than enough.

== Scheduler

The scheduler uses an array of 16 linked lists.
This structure acts like a multi-level priority queue.
Each linked list represents a priority from 0 to 15.
A lower number means a higher priority.
Priorities above 16 are invalid.

When `scheduler_insert()` inserts a task, it is inserted into the queue corresponding with its priority.

`scheduler_next()` finds the next task to run.
It returns the first unblocked task in the highest priority non-empty queue.
This task will be moved to the end of that queue, so that all tasks with the highest priority eventually get to run.

`scheduler_delete()` removes a task from the scheduler.
It simply searches through all of the scheduled tasks, and deletes one if their TIDs match.

Whenever an item is inserted into a linked list, the whole list is traversed.
Thus, if $n$ is the number of tasks currently scheduled, `scheduler_insert()` and `scheduler_next()` are $O(n)$ in the worst case.

== Context Switching

Each task has a heap-allocated Switchframe struct.
This struct stores that task's registers during a context switch.
The pointer to this struct is stored alongside the task's metadata in the task table.

Whenever we context switch, due to either a syscall or an interrupt, the kernel ensures that all of the task's registers are saved in its Switchframe.
The registers are then restored by the kernel when we return to that task.

== Message Passing

Our kernel implements the following primitives as described on the assignment page, with no changes to their signatures:

```c
int Send(int tid, const char *msg, int msglen, char *reply, int replylen)
int Receive(int *tid, char *msg, int msglen)
int Reply(int tid, void *reply, int replylen)
```

Tasks in Send() waiting for a Receive() are placed in the `SEND_WAIT` state.

Tasks in Send() waiting for a Reply() are placed in the `REPLY_WAIT` state.

Tasks in Receive() waiting for a Send() are placed in the `RECEIVE_WAIT` state.

The scheduler skips over tasks in any of the above three states.

We store pointers to messages inside the task's task table entry.
When we need to copy over a message, we use `memcpy`.

Each task table entry also contains a FIFO called the "receive queue".

When Send() is called, we check the task table to see if the target task is in RECEIVE_WAIT.
If it is, copy the message over immediately.
If not, we store the sending task's TID in the target task's receive queue.
Then, we put the sending task in SEND_WAIT.

When Receive() is called, we check if there's any tasks in the receiving task's receive queue.
If there is, copy the message from the task at the top of the queue.
If not, we simply place the receiving task in RECEIVE_WAIT.

When Reply() is called, we copy the message over immediately, unless the target task is not in REPLY_WAIT, in which we return an error.

== Name Server

The name server is created when the kernel boots up, as a child task of the init task.
It implements the following primitives as described on the assignment page, with no changes to their signatures:

```c
int RegisterAs(const char *name)
int WhoIs(const char *name)
```

The name server database is currently implemented with a linked list.
Calling `RegisterAs()` adds the task to the linked list, and calling `WhoIs()` does a linear search for the requested task.
We expect that the number of named tasks should be low, so these linear operations shouldn't incur too large a cost.

== RPS

Client tasks can play RPS with the following functions:

```c
int Signup(Tid rps)
RPSResult Play(Tid rps, RPSMove move)
int Quit(Tid rps)
```

Some notes:
 - The `rps` parameter is the TID of the RPS server. This TID can be determined using the name server.
 - `RPSResult` is a struct that stores the result of an RPS game, signifying a win, loss, draw, or that the opponent quit.
 - `RPSMove` is a struct that stores the move that the player wants to make.
 - `Signup` and `Quit` return a negative integer if there is an error, otherwise they return 0.
 - If one player quits from a game, the other player can still send in moves using Play(), but they will receive a result indicating that the opponent has quit.
 - Multiple games between different pairs of clients can be played at the same time.
 - After quitting, clients can sign up again to play another match. They may match up with a different opponent.

Rock paper scissors games are stored in a hash map with 32 buckets.
We chose 32 buckets because we expect the total number of RPS games running at any time to be lower than this.
The hash map's keys are the TIDs of the participating tasks; each game is stored twice under both TIDs.
When a player plays, the corresponding game is updated with their move.
If both players have moves recorded, the game is evaluated and the results are replied to each player.
Games are removed from the hash map when they are quit.

== AwaitEvent

AwaitEvent is implemented with no changes to its signature:
```c
int AwaitEvent(int eventType)
```

When a task calls `AwaitEvent(eventid)`, it is placed in the AWAIT_EVENT_WAIT state.
The scheduler skips any tasks in this state when choosing a task to run.

When an event occurs, the kernel will immediately unblock all tasks waiting for that event.
It then decides the next task to run using the standard metrics (round-robin on all unblocked tasks with the highest priority).

We have five events:
 - `EVENT_CLOCK_TICK` is triggered by an RPi clock interrupt, which we set to occur every 10ms. It is used by the clock server.
 - `EVENT_MARKLIN_RX` is triggered whenever the Marklin sends a byte to the RPi. It is used for Getc()'s by the Marklin IO server.
 - `EVENT_MARKLIN_CTS` is triggered whenever the Marklin changes its CTS signal to 1. We ignore interrupts where the Marklin changes its CTS signal to 0. EVENT_MARKLIN_CTS is used for Putc()'s by the Marklin IO server.
 - `EVENT_CONSOLE_RX` is triggered whenever the terminal console sends a byte to the RPi. It is used for Getc()'s by the Marklin IO server.
 - `EVENT_TASK_EXIT` is triggered whenever a task exits. It is used by the WaitTid syscall.

The usage of these events is explained more clearly in their corresponding sections below.

== Clock Server

The clock server implements the following three methods as described on the assignment page, with no changes to their signatures:

```c
int Time(int tid)
int Delay(int tid, int ticks)
int DelayUntil(int tid, int ticks)
```

The server creates a notifier task which constantly calls AwaitEvent on EVENT_CLOCK_TICK.
When it receives that event, it sends a message to the clock server.
Our clock server tracks time by counting the number of these messages.

When a task calls `Delay()` or `DelayUntil()`, the task `Send()`s a message to the clock server.
When the clock server `Receive()`s that message, it does not immediately `Reply()`.
Instead, it stores that task's Tid and the tick it should be unblocked at in a linked list.
(For `Delay(ticks)` calls, the tick to unblock at is calculated as `current_tick + ticks`).
Every tick, we loop through all tasks in this linked list, and `Reply()` to any that need to be unblocked on that tick.

The linked list might be inefficient, but that can be optimized at a later point if necessary.
For now, due to the low amount of tasks that delay at once, this is fine.

== Idle Task

This is a task which is run whenever no other tasks are running.
The kernel keeps track of what percentage of time this task is run, compared to all other tasks.
You can see this percentage in the top right of the train controller.

Whenever we context switch into the idle task, the kernel stores the current time.
Then, when we context switch out of the idle task, we use that stored time to calculate the time it spent running.

The idle task simply loops the WFI (Wait For Interrupt) instruction indefinitely, entering low power mode until an interrupt occurs.

== Virtual Machine

We have the ability to run the kernel inside Qemu.
However, Qemu is only able to simulate the Raspberry Pi 3B, so an additional compile flag `-DQemu` is used to conditionally compile the kernel for Raspberry Pi 3B compatibility.
It runs seamlessly for the most part, although interrupts are currently untested.
The simulator is useful when we are not in the lab, writing code that doesn't involve most of the devices.
It is also possible to use GDB to debug.

== IO Servers

We have two IO servers, one for the console and one for Marklin.
We believe this is better than having a single general server because it lightens the load on the Marklin IO server, ensuring it updates reactively.

Because we have two separate servers, with two separate Tid's, there is no use for the `channel` parameter in Getc() and Putc().
Just passing in the Tid of the server you want is enough.
Thus, the signatures of Getc() and Putc() are as follows:

```c
int Getc(int tid)
int Putc(int tid, unsigned char ch)
```

The console IO server only handles Getc().
Since we do a lot of debug printing, we determined that a server would be too slow for console Putc()'s.

There are three interrupts we want to detect: Marklin's CTS, Marklin's RX, and the console's RX.
Each of these interrupts has a notifier task, which repeatedly calls AwaitEvent() on that interrupt's event.

The Marklin IO server and console IO server implementations are identical, with the exception that we don't listen to console CTS's.
Calling Putc() to the console is undefined behavior.

We do not use the built-in RX and TX UART FIFOs.

=== CTS and Putc()

Every time we get a CTS interrupt, we check if there are any characters queued from previous Getc() calls.
If there are, print the oldest queued character immediately.
Otherwise, set a boolean marking that CTS is up.

Every time Getc() is called, we check that boolean to see if CTS is up.
If it is, we send the character immediately.
Otherwise, we queue the character.

Because CTS is edge-triggered, and because won't send another character until we know CTS is up, it is impossible to miss a CTS interrupt.

=== RX and Getc()

Every time we get an RX interrupt, while inside the interrupt handler we write the received character to a kernel-side queue.
We do it in the interrupt handler because we need to be quick; the character must be obtained before another character overwrites the data register.
Both the Marklin and the console have a separate kernel-side queue, to prevent them from reading each other's characters.

The interrupt handler then notifies the IO server.
The server replies to all tasks waiting on a Getc() with the next queued character.
Note that this means multiple tasks may receive the same character if they are all waiting on a Getc().
However, we expect the user to be sensible and not have multiple tasks reading from the same source.

Every time Getc() is called, we check if there are any characters in the corresponding kernel-side queue.
If there are, simply return the next queued character.
Otherwise, the task is added to a list of tasks that are waiting for a character.

== Task Names

To facilitate easier debugging of individual tasks, each task is assigned a name string.
This string's location is stored in the task's task table entry.

The `Create()` system call has been modified to take in a name:
```c
int Create(int priority, void (*function)(), const char* name)
```

There is also a new system call for querying the name of a given Tid:
```c
char* TaskName(int tid)
```

== WaitTid

`WaitTid` is a new system call:
```c
WaitTid(Tid tid)
```
It will block the calling task until the `tid` task exits.

This solves an issue surrounding `initTask`, which prompts the user for a task to run.
After you selected a task, this task would prompt the user again before the selected task had ended.

As for the implementation, `WaitTid` is simply a wrapper for `AwaitEvent`, waiting for the `EVENT_TASK_EXIT` event.
Note that the `EVENT_TASK_EXIT` event also comes with data indicating which task had exited.

== trainterm.h

To improve developer experience when writing UI, we created an ncurses-esque library.
It supports high-level manipulation of windows and terminal attributes.

This library currently does not optimize cursor movement.
We may optimize this in the future if we find that the console cannot keep up with our rendering needs.

= Bugs / Issues

- On rare occasions, the prompt will freeze up and no longer be able to take in input. We are not super sure why this happens.
- There is no logic enforcing the middle switches to not both be straight or both be curved.
- While reversing a train, the characters you type will not show up until after the train finishes reversing. As well, if you type enough characters during this window of time, the input will freeze up and no longer take any input.
