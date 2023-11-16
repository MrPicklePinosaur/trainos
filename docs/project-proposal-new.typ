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
  title: "CS452 Final Project Proposal: Train Fleets and Platoons",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "November 16th, 2023",
)

= Overview

Our goal for the final project is to enable the abilitiy for trains to travel in a 'fleet', where movement of the entire group is synchronized. This means that each train will travel behind and move as a group to the destination. At any point a time, a train is able to join the fleet or leave the fleet.

= Technical Challenges

The most obvious challenge comes with ensuring that the trains travel in sync. Experiements would need to be conducted to determine how much variance is in the acceleration, deceleration, and velocities of the each train, and if these deviations would cause issues when trains are travelling together.

The next issue would be perform proper pathfinding with a chain of trains. This will be significantly complicated if the line of trains needs to reverse (would each train reverse at a time, or would we travel reverse the entire chain as a whole?).

Finally, ideally collision safety and pathfinding should still work with other fleets and individual trains. For this to work out, we would need to take the length of the train into account and reserve larger chunks of the track.

