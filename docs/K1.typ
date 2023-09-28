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

`a42f74969b03fbfb5fbd11cc997a4f9dfb87b9d6`

= Kernel Features

If you run the kernel, it will print the OS's logo (TRAINOS), and then run the first user task, which operates as described on the assignment page.
The kernel then hangs.
There is currently no way to start any other task without modifying the code.

Our kernel implements the five syscalls described on the assignment page with no modifications to their signatures:

```c
int Create(int priority, void (*function)())
```

```c
int MyTid()
```

```c
int MyParentTid()
```

```c
void Yield()
```

```c
void Exit()
```

The first task created has a TID of 1.
No task can have a TID of 0.
A TID of 0 acts as an error value for functions that should return a TID but cannot.

Our kernel properly schedules tasks, using a round robin algorithm to cycle through all tasks of the highest priority.

Our kernel theoretically allows for the creation of 1024 tasks, though this is currently bottlenecked by our fake heap allocator.

When there are no more tasks in the scheduler, our kernel simply hangs.

= Kernel Implementation

== Memory Layout

We allocate 2MB for the kernel stack, and each task gets a 1MB page to use as a stack.
The maximum amount of tasks is 1024 so that we take up at most 1GB (+ 2MB for the kernel) of memory.
We currently do not reclaim pages.
Thus, we can only create 1024 tasks over the lifespan of the kernel.
If this limit is reached, the kernel prevents the creation of new tasks.

We currently have a fake heap allocator implemented, which is actually 2048 bytes of static memory.
This will be improved into an actual heap allocator at a later point in time.
Unfortunately, this means that we run out of memory on the sixth task created.
Fortunately, the assignment only requires five.

== Task Table

Task metadata is stored in a hash table, which is implemented as an array of 128 linked lists.
We use linked lists so that we can store multiple tasks with the same hash.
The hash of a task is simply its TID modulo 128.
Adding a task inserts it into the corresponding linked list.
When we need to retrieve a task, we search for it inside the correspoding linked list.
Theoretically, a hash map implemented this way will start operating slowly if many tasks are run, but for our use cases, there are enough hash keys that speed shouldn't be a problem.

== Scheduler

The scheduler uses a multilevel "queue" with 16 levels.
Queue is in quotations because it supports the deletion of any item within the queue, something a queue cannot typically do.
Each queue represents a priority from 0 to 15. A lower number means a higher priority.
Priorities above 16 are invalid.

When scheduler_insert() inserts a task, it is inserted into the queue corresponding with its priority.

scheduler_next() finds the next task to run.
It returns the next task in the highest priority non-empty queue.
This task will then be moved to the end of that queue, so that all tasks with the highest priority eventually get to run.

scheduler_delete() will remove a task from the scheduler.
It simply searches through all of the scheduled tasks, and deletes one if their TIDs match.

The multilevel queue is implemented as an array of linked lists.
Whenever an item is inserted into a linked list, the whole list is traversed.
Thus, if $n$ is the number of tasks currently scheduled, scheduler_insert() and scheduler_next() are $O(n)$ in the worst case.

== Heap allocator

Our heap allocator is currently just 2048 bytes of static memory.
Memory is never freed, so we run out of memory really fast.
Thankfully, this is enough bytes to complete this assignment with.

= Program Output

The program produces the following output:

```
Created: 2
Created: 3
Created: 4
Created: 5
FirstUserTask: exiting
MyTid = 4, MyParentTid = 1
MyTid = 5, MyParentTid = 1
MyTid = 4, MyParentTid = 1
MyTid = 5, MyParentTid = 1
MyTid = 2, MyParentTid = 1
MyTid = 3, MyParentTid = 1
MyTid = 2, MyParentTid = 1
MyTid = 3, MyParentTid = 1
```

Our program starts with task 1, which has a priority of 4.
Task 1 creates tasks 2 and 3, of priority 5, and tasks 4 and 5, of priority 3 (recall that lower numbers mean higher priorities).

The reason the output is in this specific order is because tasks 4 and 5 have higher priorities than task 2 and 3.
This means that they will run first, and only when both of them have exited (or both of them get blocked) do the lower priority tasks run.

The scheduler alternates between tasks 4 and 5 (and later between tasks 2 and 3) because it employs a round robin algorithm that cycles between all tasks of the highest priority.
This is to ensure that a task of the highest priority does not get starved.
