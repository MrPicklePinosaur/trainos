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

When a task calls AwaitEvent(), it blocks until the event occurs.

When an event occurs, the kernel will immediately unblock all tasks waiting for that event.
The kernel will then find the next task to run in the scheduler.
We do not return to the same task that was running, because we have a constantly looping init task

= Kernel Implementation

== Clock Server

The notifier task constantly calls AwaitEvent, waiting for the next clock tick.
After each tick, it sends a message to the clock server.
Our clock server simply counts up the amount of these ticks.
Every tick, it replies to all tasks that need to be unblocked on that tick.
`Delay(ticks)` is simply implemented as `DelayUntil(current_tick + ticks)`.

All tasks currently waiting for Delay() or DelayUntil() are stored in a linked list.
We simply traverse the list when we want to check which tasks to unblock each tick.

= K3 Output

The output would thus follows the following sequence:

#table(
  columns: (auto, auto, auto, auto, auto),
  [1], [10], [], [], [],
  [1], [20], [], [], [],
  [2], [], [23], [], [],
  [1], [30], [], [], [],
  [3], [], [], [33], [],
  [1], [40], [], [], [],
  [2], [], [46], [], [],
  [1], [50], [], [], [],
  [1], [60], [], [], [],
  [3], [], [], [66], [],
  [2], [], [69], [], [],
  [1], [70], [], [], [],
  [4], [], [], [], [71],
  [1], [80], [], [], [],
  [1], [90], [], [], [],
  [2], [], [92], [], [],
  [3], [], [], [99], [],
  [1], [100], [], [], [],
  [1], [110], [], [], [],
  [2], [], [115], [], [],
  [1], [120], [], [], [],
  [1], [130], [], [], [],
  [3], [], [], [132], [],
  [2], [], [138], [], [],
  [1], [140], [], [], [],
  [4], [], [], [], [142],
  [1], [150], [], [], [],
  [1], [160], [], [], [],
  [2], [], [161], [], [],
  [3], [], [], [165], [],
  [1], [170], [], [], [],
  [1], [180], [], [], [],
  [2], [], [184], [], [],
  [1], [190], [], [], [],
  [3], [], [], [198], [],
  [1], [200], [], [], [],
  [2], [], [207], [], [],
  [4], [], [], [], [213],
)
