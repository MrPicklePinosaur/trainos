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
  title: "CS452 Final Project Proposal",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "November 16th, 2023",
)

= The future of TrainOS

Up until now, we have been limited to our tiny world of the raspberry pi. There
is no interaction  between the pi and the outside world. This is boring and sad.
Thus, we propose to unlock the potential for TrainOS to communicate to
arbitrary devices across the network. In doing so, not only do we enable the
ability for remote servers to understand the state of the trains and the track,
it also allows the ability to remotely send data to and control the Marklin.

So potential applications of a functional networking stack include:
- remote simulation of the track (render the state of the track and trains on a UI on your own computer)
- in the same vein, an AR application that has the ability to render 3D models onto the track, as well as with real time manipulation of the track in 3D space (point to switch to toggle it etc)
- remote control of the Marklin, example from a smart phone or web ui, which allows the development of more complex user interfaces that are not just limited to sending bytes across UART to the terminal
- in fact, we can even accomplish synchronization between track A and track B, one project could be to design a system that would replicate the state of one track to the other, resulting in train synchronization

In short, we seek to bring our kernel into the age of the internet and build something cool using that.

= Implementation

First we will need a way to communicate with ethernet, although wireless / bluetooth is also possible

We need a rudimentary networking stack, with a DNS server to facilitate domain name lookups, and a simple http library.

= Challenges

How hard could it be

