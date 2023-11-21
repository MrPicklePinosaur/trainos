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
  title: "CS452 TC2",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "TBD",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: `TBD`

= Stuff



= Short Moves

Short moves are measured manually.
We start with the train stopped.
We then set the train to speed 8, and wait x milliseconds.
We then stop the train, and use a tape measure to determine how far it moved.

We start x at 250ms and we increase it by 250ms each test, up to 4000ms.
Measuring every 250ms gives us more than enough granularity, and measuring up to 4000ms lets us cover the largest possible short move (from C13 to E7).

We use linear interpolation to determine give us some more accuracy with short moves.
As well, we can also extrapolate if a short move is longer than the longest measured distance.

= Sensor Attribution

At the start of the program, for each train, we start the train moving at a slow speed and wait for it to hit a sensor.
This gives us the initial position and direction of each train.
After this, whenever the train moves, we use the track graph as well as the position of the switches to predict the next sensor the train should hit.

We currently assume that this predicted sensor will only be hit by that specific train.

= Track Reservation

Our current algorithm reserves every zone on the train's path.
Then, as the train exits each zone, the zone is freed.

= Zoning

We treat each sensor as the edge of a zone.
This is simply for convenience.
This gives us 26 zones in total:

#image("zones.png", width: 100%)
