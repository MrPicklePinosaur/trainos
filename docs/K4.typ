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

Commit SHA: `To be decided`

= Running The Train Controller

After booting up the kernel, you will be prompted to `SELECT TASK TO RUN`.
Choose a task by entering a number on the keyboard.
The train controller task is the one called `K4`.

= Kernel Features

== Marklin CTL

The marklin control TUI from assignment 0 was ported to `trainos` and makes extensive use of tasks and message passing to achieve lower latency in updating data and rerendering the UI. The control TUI provides a prompt to enter commands to the marklin, diagonistic information, recently triggered sensors, as well as table for switch state.

The supported commands are as follows:

`tr <train number> <speed>`
Controls the speed of a given train, where `<train number>` is between 1 and 80, and speed is between 0 and 14

`rv <train number>`
Reverses the direction a given train is going

`sw <switch number> <switch direction>`
Sets the state of a switch. `<switch direction>` must be "S" (for straight) or "C" (for curved)

`light <train number> <on|off>`
Toggle the lights on the train on and off

`go`
Turns on the marklin

`stop`
Turns off the marklin

`q`
Quits the program

Note that the commands are case-sensitive. In particular, the `S` or `C` passed to `sw` must be capitalized.

== IO Servers

We have two IO servers, one for the console and one for Marklin.
We believe this is better than having a single general server because it lightens the load on the Marklin IO server, ensuring it updates reactively.

Because we have two separate servers, with two separate Tid's, there is no use for the `channel` parameter in Getc and Putc.
Just passing in the Tid of the server you want is enough information.
Thus, the signatures of Getc and Putc are as follows:

```c
int Getc(int tid)
int Putc(int tid, unsigned char ch)
```

The console IO server only handles Getc.
Since we do a lot of debug printing, we determined it would be too slow using a server for console Putc's.

== Task Names

To facillitate easier debugging of individual tasks, each task now can be assigned a name string. The name is stored in the task data structure and the `Create()` systemcall has been modified to take in a name.
```c
int Create(int priority, void (*function)(), const char* name);
```

A task name system call has been introduced for any task to query the name of a given Tid.
```c
char* TaskName(int tid);
```

== WaitTid

An additional system call was added in this version of the kernel. Previously we would have an issue that `initTask`, which was responsible for prompting the user for a task to run, was prompting the user before the previously ran task would finish running. To solve this, we introduced a system call that will block the calling task until the desired task has exited. As for implementation, `WaitTid` is actually just a wrapper around `AwaitEvent`.

== trainterm.h

To improve developer experience when writing UI, an ncurses-esque library was created. It supports higher level manipulation of windows and terminal attributes. Currently no cursor movement optimizations are made. This may be a consideration if we find that the console cannot keep up with out rendering needs.

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
We do it in the interrupt handler because we need to be quick; we need to get the character before another character overwrites the data register.
The IO server then gets notified that RX was called, and all tasks waiting on Getc() receive that character.

This means that if multiple tasks are waiting on a Getc() from the same source, they all get the same character.
However, we do not expect the user to have multiple tasks waiting on a Getc().

Every time Getc() is called, we check if there are any characters in the kernel queue.
If there are, simply return the oldest queued character.
Otherwise, the task gets added to a list of tasks waiting for a character.
