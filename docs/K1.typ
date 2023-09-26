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
  title: "CS452 K1",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "September 28, 2023",
)

= Commit SHA

To be added

= Kernel Features

= Kernel Implementation

== Task Table

Tasks are stored in a simple array that can hold 128 items.

== Scheduler

The scheduler uses a multilevel "queue" with 16 levels.
Queue is in quotations because it supports the deletion of any item within the queue, something a queue cannot typically do.
Each queue represents a priority from 0 to 15. A lower number means a higher priority.
Priorities above 16 are invalid.

When scheduler_insert() inserts a task, it is inserted into the queue corresponding with its priority.

scheduler_next() finds the next task to run.
It returns the next task in the highest priority non-empty queue.
This task will then be moved to the end of that queue, to provide fairness between all tasks of the highest priority.

scheduler_delete() will remove a task from the scheduler.
It simply searches through all of the tasks, and deletes one if their ids match.

The multilevel queue is implemented as an array of linked lists.
Whenever an item is inserted into a linked list, the list whole list is traversed.
Thus, if $n$ is the number of tasks currently scheduled, scheduler_insert() and scheduler_next() are $O(n)$ algorithms.
