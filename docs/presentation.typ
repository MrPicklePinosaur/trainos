
#import "@preview/polylux:0.3.1": *

#let trainos-theme(aspect-ratio: "16-9", body) = {
  let background-color = navy

  set page(
    paper: "presentation-" + aspect-ratio,
    fill: background-color,
  )
  set text(fill: white.darken(10%), size: 40pt, font: "Fira Sans")

  body
}
#show: trainos-theme.with()

#set raw(theme: "trainos.tmTheme")
#show raw: it => block(
  text(fill: rgb("#ffffff"), it)
)

#set page(paper: "presentation-16-9")
#set text(size: 25pt)

#polylux-slide[
  #align(horizon + center)[
    = TrainOS

    Daniel Liu and Joey Zhou
  ]
]


#polylux-slide[
    = Directory Structure

    ```
    .
    ├── include/
    │   └── ...
    ├── kern/
    │   └── ...
    ├── lib/
    │   ├── trainstd/
    │   ├── trainsys/
    │   └── trainterm/
    └── user/
        └── tests/
    ```
]

#polylux-slide[
    = Tasks & Task Table
]

#polylux-slide[
    = Address space
]

#polylux-slide[
    = Context Switch
]

#polylux-slide[
    = Memory Allocation

    Free list based malloc for pages and arena allocations for actual use
]

#polylux-slide[
    = Kernel Events
]

#polylux-slide[
    = Essential tasks

    - Nameserver
    - Clock  
    - IO (one for Marklin, one for Console)
    - Idle
]

#polylux-slide[
    = MarklinCTL TUI


]

#polylux-slide[
    = Testing

    Test harness for running all of your tests
]

#polylux-slide[
    = Logging

    Multiple log levels and log masks to filter output
]

#polylux-slide[
    = QEMU

    Only supports up to RPi 3B.

    Use some conditional compilation to work with RPi 3B memory layout.

    Interrupts don't work D:
]

#polylux-slide[
    = Gacha

    :D
]

#polylux-slide[
    = trainstd.h: the standard library

    Contains data structures like linked list, hashmap, circular buffer and more.
]

#polylux-slide[
    = trainterm.h: ncurses inspired terminal graphics

    Primitive terminal drawing library with ncurses inspired API. Currently no cursor movement optimizations.
    ```c
    Window win = win_init(2, 2, 20, 10);
    win_draw(&win);

    w_attr(ATTR_RED);
    w_puts_mv("my window", 2, 2);
    w_attr_reset();
    ```
]

#polylux-slide[
    = Funny bugs
]

