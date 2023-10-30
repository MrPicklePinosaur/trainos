
#import "@preview/polylux:0.3.1": *

#set page(paper: "presentation-16-9")
#set text(size: 25pt)

#polylux-slide[
  #align(horizon + center)[
    = TrainOS

    Daniel Liu and Joey Zhou
  ]
]

#polylux-slide[
    = Kernel Design
]

#polylux-slide[
    = Memory Allocation

    Free list based malloc for pages and arena allocations for actual use
]

#polylux-slide[
    = Essential tasks

    - Nameserver
    - Clock  
    - IO (one for Marklin, one for Console)
    - Idle
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
    = trainstd.h: the standard library

    Contains data structures like linked list, hashmap, circular buffer and more.
]

#polylux-slide[
    = trainterm.h: ncurses inspired terminal graphics

    Primitive terminal drawing library with ncurses inspired API. Currently no cursor movement optimizations.
]

