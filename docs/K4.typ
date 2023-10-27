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

= Running The Train Controller

After booting up the kernel, you will be prompted to `SELECT TASK TO RUN`.
Choose a task by entering a number on the keyboard.
The train controller task is the one called `K4`.

= Kernel Features

== Marklin CTL

The Marklin control TUI from Assignment 0 was ported to `trainos`.
It uses tasks and message passing to lower the latency of data updates and UI re-renders.
The UI includes a prompt to enter commands, diagnostic information, a list of recently triggered sensors, and a table for switch states.

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

== IO Servers

We have two IO servers, one for the console and one for Marklin.
We believe this is better than having a single general server because it lightens the load on the Marklin IO server, ensuring it updates reactively.

Because we have two separate servers, with two separate Tid's, there is no use for the `channel` parameter in Getc() and Putc().
Just passing in the Tid of the server you want is enough information.
Thus, the signatures of Getc() and Putc() are as follows:

```c
int Getc(int tid)
int Putc(int tid, unsigned char ch)
```

The console IO server only handles Getc().
Since we do a lot of debug printing, we determined it would be too slow using a server for console Putc()'s.

== Task Names

To facilitate easier debugging of individual tasks, each task is now assigned a name string.
This string's location can be found in the task's tasktable entry.

The `Create()` systemcall has been modified to take in a name:
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
After you selected a task, this task would prompt the user again before the selected task ended.

As for the implementation, `WaitTid` is simply a wrapper for `AwaitEvent`.

== trainterm.h

To improve developer experience when writing UI, we created an ncurses-esque library.
It supports high-level manipulation of windows and terminal attributes.

This library currently does not optimize cursor movement.
We may optimize this in the future if we find that the console cannot keep up with our rendering needs.

= Kernel Implementation

== IO Servers

There are three interrupts we want to detect: Marklin's CTS, Marklin's RX, and the console's RX.
We use a notifier pattern to listen to each of these.
That is, each has an event, and each has a notifier task which repeatedly calls AwaitEvent().

The Marklin IO server and console IO server implementations are identical, with the exception that we don't listen to console CTS's.
Calling Putc() to the console is undefined behavior.

=== CTS and Putc()

Every time we get a CTS interrupt, we check if there are any characters queued from previous Getc() calls.
If there are, print the oldest queued character immediately.
Otherwise, set a boolean marking that CTS is up.

Every time Getc() is called, we check that boolean to see if CTS is up.
If it is, we send the character immediately.
Otherwise, we queue the character.

Because CTS is edge-triggered, and because won't send another character until we know CTS is up, it is impossible to miss a CTS interrupt.

=== RX and Getc()

Every time we get an RX interrupt, we write the character to a kernel-side FIFO while inside the interrupt handler.
We do it in the interrupt handler because we need to be quick; the character must be obtained before another character overwrites the data register.
The IO server is then notified that an RX interrupt occurred, and replies to all tasks waiting on a Getc() with the oldest queued character.
Note that this means multiple tasks may receive the same character if the all Getc() at once.
However, we expect the user to be sensible and not create multiple tasks reading from the same source.

Every time Getc() is called, we check if there are any characters in the kernel queue.
If there are, simply return the oldest queued character.
Otherwise, the task gets added to a list of tasks waiting for a character.

= Bugs / Issues

- On rare occasions, the prompt will freeze up and no longer be able to take in input. We are not super sure why this happens.
- There is no logic enforcing the middle switches to not both be straight or both be curved.
- While reversing a train, the characters you type will not show up until after the train finishes reversing. As well, if you type enough characters during this window of time, the input will freeze up and no longer take any input.
