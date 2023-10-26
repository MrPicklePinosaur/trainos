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

= Kernel Features

== Running The Train Controller

After booting up the kernel, you will be prompted to `SELECT TASK TO RUN`.
Choose a task by entering a number on the keyboard.
The train controller task is the one called `K4`.

The train controller supports the `tr`, `sw`, and `rv` commands from the first assignment.
Note that the commands are case-sensitive.
In particular, the `S` or `C` passed to `sw` must be capitalized.

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
