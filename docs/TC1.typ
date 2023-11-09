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
  title: "CS452 TC1",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "TBD",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: `TBD`

= Measurement Methodology

To measure velocity of the train at certain speeds, we run it on the inner loop of Track A (that is, the one that goes C10 B1 D14 E14 E9 D5 E6 D4 B6 C12 A4 B16).
We measure the time between each of the 12 sensors on the track, and so each loop gives us 12 samples.
We loop the train 20 times, thus giving us a total of 20*12=240 samples.

To measure stopping distance, we run a binary search.
We wait for the train to hit sensor C10, wait $t$ microseconds, then send the stop command.
If the the train hits sensor E14, we waited too long, and so we decrease $t$.
If the train doesn't hit it, we didn't wait long enough, and so we increase $t$.

Calibration data is stored in a plaintext file `docs/measurements.txt`.
We manually export this data into an array of constants in `user/path/train_data.h`.

= Shortest Path

We implement Dijkstra's algorithm to find the shortest path between two sensors.
In our implementation, we assume we can't reverse the train using a reverse command (since we would need to consider the train's reversal time, complicating our algorithm).
However, we do treat both directions of a sensor as the same.
For example, A3 and A4 are the same sensor but point in different directions.
We treat pathfinding to A3 the same as pathfinding to A4.

= Stopping The Train

Dijkstra returns an array of edges from the track graph, in the order they need to be traversed.
We start at the end of this array and traverse backwards, looking for the first sensor that is further from the destination than the stopping distance of the train.
We calculate $t_s$ the length of time after that sensor is triggered before sending the stop command.
Then, we wait for the train to trigger that sensor, wait for $t_s$ to pass, and then send the stop command.
This should stop the train near the destination.

If the destination is closer to the train than the stopping distance of the train, we pathfind assuming that the train will pass the sensor.
This typically makes the train loop around the track.

= Kernel Changes

= Known Bugs



