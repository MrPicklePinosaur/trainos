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
  title: "CS452 Final Project Proposal: Fleets",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "November 16th, 2023",
)

= Overview

Our goal for the final project is to allow trains to travel in a 'fleet'.
A fleet is a group of trains travelling together in a single file line.
At any point in time, a train may join the fleet from either end.
As well, a train on either end of the fleet may leave the fleet.
If time permits, we can also implement the ability for trains in the middle of the fleet to leave.

= Technical Challenges

While it would be nice if all the trains could move at the same velocity, the reality is that their velocities vary wildly.
Some trains would need to rhythmically speed up and slow down to simulate constant velocity.
But we also need to ensure that this doesn't cause adjacent trains to bump into each other.
Our calibrated train data can give us information on how to accomplish this speed adjustment.
As well, some live calibration is likely crucial in ensuring the trains are spaced apart well enough.

Another problem is getting the fleet to start, stop, and reverse together.
Since the trains have different stopping times, we cannot just tell them all to stop at the same time.
We might need to slow some trains down before stopping them, and tell them to stop at different times.
Variance in the acceleration, deceleration, and velocities of each train may complicate this further.

Pathfinding is a little more complicated with a chain of trains.
If the fleet is long enough, it may deadlock itself if it travels in a loop.

Finally, ideally collision safety should work between fleets and other fleets, and between fleets and single trains.
This means that the length of a fleet is now a consideration when reserving track zones.
