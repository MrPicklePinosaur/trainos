
#import "@preview/polylux:0.3.1": *

#set page(paper: "presentation-16-9")
#set text(size: 25pt)

#polylux-slide[
  #align(horizon + center)[
    = TrainOS

    Joey Zhou and Daniel Liu

  ]
]

#polylux-slide[
    = Kernel Design
]

#polylux-slide[
    = Memory Allocation

    Free list based malloc for pages and arena allocations for actual use
]
