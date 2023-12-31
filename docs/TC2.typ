#let project(title: "", authors: (), date: none, body) = {
  set document(author: authors, title: title)
  set page(numbering: "1", number-align: center)
  set text(font: "New Computer Modern", lang: "en")
  show math.equation: set text(weight: 400)

  align(center)[
  
    #image("logo-colored.png", width: 40%)
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
  date: "November 26th, 2023",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: e55e4a8b2d55c98545e804d85b3466d7bcec5a23

= Train Movement

== Reversals

Our pathfinding now considers reversals.
Dijkstra will travel along reverse edges, treating them as length 0 edges.
Reversing is not instant, so this assumption is inaccurate, but we haven't implemented a better alternative yet (simply setting the edge length to non-zero breaks a few things).

One challenge with train reversal is that sometimes the train comes to a stop halfway on top of a switch, causing it to derail when it reverses.
Our solution is to stop at the first sensor after the switch before reversing, which (in most cases) guarentees the train is in a safe position before reversing.

Reversals are implemented by pathfinding to the point of reversal, reversing, and then doing another pathfind to the next destination.

== Short Moves

Short moves were measured manually.
We started with the train stopped.
We set the train to speed 8, then waited x milliseconds before stopping the train.
The distance the train travelled was measured using a tape measure.

The variable x started at 250ms and was increased by 250ms each test, up to 4000ms.
Measuring every 250ms gave us a confident amount of granularity.
As well, going up to 4000ms let us handle the largest possible short move (from C13 to E7).

For distances between our measured distances, we use linear interpolation to estimate the wait time.
For distances longer than our longest measured distance, we use linear extrapolation from the last two measurements to estimate the wait time.



= Calibration

== Automatic Calibration Mode

In automatic calibration mode, each train determines its location by moving at a slow speed until it hits a sensor.
We assume that the trains tarts at that sensor.
If a sensor is not tripped after a certain amount of time, we determine that the train is not on the track.

This method works great but takes a long time to do.
We turn it off during testing, because manual setup is much faster.

Another problem with this mode is that the trains don't stop exactly on top of the sensors; they take some time to decelerate.
This overshooting can lead to the train poking into the next zone, which has the possibility of causing a collision.

== Manual Calibration Mode

In manual calibration mode, the user must input the train's current sensor by using the `pos` command.
This method is sensitive to user error; getting the sensor direction wrong is a pretty common one.



= Sensor Attribution

We have two different algorithms for sensor attribution; they can be swapped via a recompile.

== Zone local attribution

This is the attribution algorithm that we used in the demo.

When a sensor is hit, if there is a train in the same zone as the sensor, we attribute it to that train.
If there is no train within the zone, we consider the sensor spurious.

We split up zones such that all adjacent sensors (that is, sensors that have no sensors separating them) are in the same zone.
Thus, the 3 (or more) sensors around a branch (or series of branches) are all in the same zone, which means we correctly attribute sensors even if the train goes down the wrong branch.

Should we fail to find the sensor for a train, we also check sensors in the previous zone in case the train unexpectedly reversed.

== Dijkstra attribution

When a sensor is triggered, we run Dijkstra from it to each train, and attribute the sensor to the closest train.
This method works quite well but is costly to compute, and leads to much slower response times for sensors.
It also does not handle spurious sensors well.



= Track Reservation

Our current algorithm reserves every zone on the train's path.
Then, as the train moves along the path, it frees zones as it exits them.

Our pathfinding will try to find a path that avoids reserved zones.
However, if this is impossible, we do a partial move.
That is, we find the fastest path disregarding reservations, and move the train partway along this path, stopping one zone away from the reserved zone.
It then waits for that zone to be unreserved before continuing.

This method does not guarantee deadlock avoidance.
Overall, we have no way to avoid deadlocks.



= Zoning

Our zones are bounded by sensors, and every sensor is a zone boundary.
For track A, this gives us 30 zones:
#image("zones.png", width: 100%)

This is as fine-grained as we can get with sensor-based division.
Smaller zones reduce our chances of deadlock and give more potential for simultaneous train movement.
However, there is increased collision potential since trains can end up much closer to each other.

Zones are stored in a data structure with a list of pointers to the sensor nodes that bound the zone.
There is also a list of pointers to the merge and branch nodes in the zone.



= Pathfinding

Each `path` request spawns a new `Pather` task, which coordinates the train commands, switch activations, and sensor waits for that path.
To cancel a path, we use our new `Kill()` syscall to stop the `Pather`.

== Switch activation

We set all of the switches in a zone when the train enters the previous zone.
This ensures that there is one zone of buffer before the train arrives at the switch.
Without this buffer, switches sometimes activated too late, causing wrong turns and derailings.



= Collision prevention

We have not implemented any form of emergency stop if two trains somehow get into the same zone.



= Servers

The following servers were introduced in TC2:

== Trainstate server

This is the single source of truth for the state of each train.
It stores the speed, orientation, and position of the trains.
Any changes to train state must be done through this server, guaranteeing mutual exclusion.

A notifier alerts the server whenever a sensor triggers, running the sensor attribution algorithm and determining the current position of the train.

This server also allows a task to block until a train changes position.
We use this functionality extensively in our pathfinding algorithms.

== Reservation server

This server tracks which trains have reserved which zones.
It has a simple reserve/unreserve interface that is idempotent (allowing a train to reserve the same zone multiple times without error).

== Trainterm (render) server
 
In our UI, each window is its own task.
This means multiple tasks are trying to print to the console at once, which can lead to issues if the task is interrupted in the middle of a print.
Cursor positioning gets messed up, and the resulting visual bugs can render the UI unreadable.

We introduced the render server to guarentee mutual exclusion on the cursor.
Each window maintains it's own output buffer and writes into it.
The ouput buffer is only flushed upon calling `w_flush()`, where the entire buffer is sent to the render server to be rendered atomically.

The render server processes these requests in FIFO order.
