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
  date: "November 9, 2023",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: `TBD`

= Running Train Control 1

To test the new TC1 features, select the task called `K4` in the kernel task menu.
This will run our train TUI.

To route trains, use the following command:
```c
path <train number> <destination> <train speed> <offset (optional)>
```

`destination` will be the name of the node you want the train to go to, such as `A3`, `C10`, or `BR5`.
Opposite nodes are equal, so `A3` is the same as `A4` and `BR5` is the same as `MR5`.

`train speed` can be one of 5, 8, 11, or 14.

`offset` is how many millimeters away from the node you would like to stop.
It defaults to 0.
The distance cannot be so large that it takes you past another node.
For example, if `C10` comes 150mm before `BR16`, you can route to `C10` with an offset of 149mm, but not 151mm.

We make no distinction between opposite nodes, so `A3` and `A4` are treated as the same.
Although, the offset is based on the direction the train is going.
So `A3` with an offset of 200mm will stop either 200mm in front of or behind `A3`, depending on if the train is routed to it in the direction of `A3` or `A4`.

A new window has been added to our TUI which predicts when the next sensor will be triggered.
This window is in the top right of the TUI.

= Measurement Methodology

To measure velocity of the train at a certain speed, we run it on the inner loop of Track A (that is, the one that hits sensors in the order C10 B1 D14 E14 E9 D5 E6 D4 B6 C12 A4 B16).
We measure the time between each pair of consecutive sensors.
A full loop has 12 sensors, and thus gives us 12 samples.
We run 20 loops, so our total amount of samples is 20*12=240.
We believe this is more than enough samples to give us an accurate train velocity.

To measure stopping distance, we run a binary search.
In each iteration of the search, the program waits for the train to hit sensor `C10`, then waits $t$ microseconds, then sends the stop command.
If the the train hits sensor `E14`, the wait is too long, and $t$ is decreased.
If after six seconds, the train doesn't hit it, the wait is too short, and $t$ is increased.
The train is then looped around the track to automatically repeat this experiment.
We iterate the binary search 20 times, which gives us an accuracy of less than a microsecond.

Since we know the distance between `C10` and `E14` is 1045mm, we can then calculate the stopping distance as $1045 - v t$.

This method does not account for differences in sections of the track.
As well, it is liable to variations in stopping distance between iterations.
We can smooth out the latter issue by doing the binary search multiple times.
For now, we have only done one binary search for each train speed.

The raw calibration data can be found in the plaintext file `docs/measurements.txt`.
We manually export this data into an array of constants in `user/path/train_data.h`.

Calibration is done using a program separate from our kernel.
It uses a polling loop, and is implemented as barebones as possible in order to reduce the polling loop time.
You can find this program in the `calib` folder in our repository.

= Shortest Path

Our pathfinding uses Dijkstra's algorithm to find the shortest path between two nodes.
This algorithm does not factor in train reversing yet, since we would need to measure the train's reversing time for it to be accurate.

We determine the starting node of the pathfinding by waiting for a sensor after the `path` command is sent.
The first sensor triggered gives us the direction and position of the train.
We run Dijkstra from this sensor, and then switch all the necessary switches immediately after.

= Stopping The Train

Dijkstra returns an array of edges from the track graph, in the order they need to be traversed.
We traverse backwards from the end of this array, searching for the first sensor that is further from the destination than the train's stopping distance.
Then, using distance and train velocity information, we calculate $t_s$ the length of time after that sensor is triggered before we send the stop command.

Once the train triggers that sensor, we wait for $t_s$ to pass, and then send the stop command.
This should stop the train near the destination.

= Servers

We have implemented several new servers to help with train control.
We have one server for tracking train speeds and predicting train locations, one for detecting sensor data, and one for tracking switch positions.

Any tasks can get train data or sensor data or switch data by sending messages to these servers.

= Kernel Changes

We have implemented a `Puts()` call in the IO server.
This allows us to send a series of commands to the Marklin atomically.
Notably, this means that being interrupted between bytes in a multi-byte command is no longer an issue.

= Known Bugs

There are issues pathfinding to a destination too close to the train (i.e. within stopping distance).
We plan to support this case by running Dijkstra starting from a sensor after the destination.
