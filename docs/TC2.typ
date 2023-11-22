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
  date: "November 22nd, 2023",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: `TBD`

= Train Movement

== Reversals

Dijkstra was modified to also consider the reverse of the node when exploring neighbour nodes. The difficulty with train reversal is that we need to ensure that the train is not parked ontop of a switch when the switch changes state. To remedy this, dijkstra guarentees that the node a reversal occurs on must be a sensor mode, which (in most cases), guarentee the train to be in a safe position.

The pathfinding algorithm will then break a path into simple paths that contain no reversals. Thus, the train will pathfinding and stop at the point of the reversal, then receive the reverse command, and continue to the next destination. This was done as our original method of train reversal makes no guarentee on where the reversal point of the train was.

== Short Moves

Short moves are measured manually.
We start with the train stopped.
We then set the train to speed 8, and wait x milliseconds.
We then stop the train, and use a tape measure to determine how far it moved.

We start x at 250ms and we increase it by 250ms each test, up to 4000ms.
Measuring every 250ms gives us more than enough granularity, and measuring up to 4000ms lets us cover the largest possible short move (from C13 to E7).

We use linear interpolation to determine give us some more accuracy with short moves.
As well, we can also extrapolate if a short move is longer than the longest measured distance.

= Calibration

== Automatic Mode

As part of the initalization process, each train callibrates itself. It does this by moving at a slow speed until a sensor is hit, and then stopping immediately. This way without any human intervention we are able to determine the orientation (assuming that trains always start moving forward) and position of each train. It is also possible to determine which trains are placed onto the track by using a timeout after sending the callibration command, and if the timeout expires, the train is not on the track.

This method is great but it does take some time during startup, unideal when doing rapid prototyping. Furthermore, due to stopping delays, trains will always overshoot the sensor by some distance after calibration, which may lead to the train running into the next zone. Due to these limitations, a manual calibration mode was also introduced.

== Manual Mode

In manual calibration mode, trains do not calibrate themselves. Instead the user uses the prompt (the `pos` command) to input the starting locations of each train themselves. This method has obvious user error issues, especially getting the sensor directions wrong.

= Sensor Attribution

There are three different algorithms implemented for sensor attribution that can be swapped around via a recompile. We experimented a bit with different schemes to see what gives the best results.

== Zone local attribution

In zone local attribution, for each train we see if the newly hit sensor falls into the zone of the train. Due to how our zones are formed (see `Zoning` section), all sensors in a zone are adjacent (have no sensors sepratating them). This naturally helps account for cases where the train goes down the wrong branch.

If no sensors were able to be assigned, we also look in the previous zone to account for cases where the train unexpectedly reverses.

This is currently the default attribution algorithm.

== Dijkstra attribution

The next attribution algorithm runs dijkstra from the new sensors trigger to the locations of each train and chooses the closest node to give the sensor activation to.

This method worked quite well but was quite costly to compute, we saw much slower response times for sensors.

= Track Reservation

Our current algorithm reserves every zone on the train's path.
Then, as the train exits each zone, the zone is freed.

= Zoning

The track is divided into zones by using sensors as boundaries. This is perhaps the most fine grain sensor division possible, reducing many chances of deadlocks and allowing more chances for trains to move at the same time. This of course also introduces many challenges and increases the risk of collision.

Zones are stored in a seperate data structure with the boundaries of the zone being pointers to the original track nodes. Pointers to switches in the zone are also kept.

For track A, we have 26 zones, as shown below:
#image("zones.png", width: 100%)

= Pathfinding

After a path request is submitted via the prompt or programatically, a new `Pather` task is spawned that coordinates the sequence of train commands, switch activations, and sensor waits that complete the desired path.

== Zone reservation

A walk is done through the entire path and each zone is attempted to be reserved. This means that the first train will reserve the entire path. This method is beneficial as future path requests will be mindful of the existing path and try to work around it, reducing the possibility of collisions, deadlocks, and waiting around.

In the case that a zone reservation fails, the train will traverse down the *partial path* that has been allocated and then wait until the desired zone becomes free. This is done so that if a train's desired path is blocked, it will move as much as it can to reduce travel time once the zone is free again.

== Zone freeing

Upon entry of a new zone, the current zone is immediately freed. This is a safe operation as any trains that claim this zone will be waiting to enter from at least a distance of one zone away, and from rest, meaning that it will take substantial time to enter the newly freed zone after the original train has exited. The possibility of two trains being in a newly freed zone is very low, while also allowing other trains to claim the zone earlier.

== Switch activation

Upon entry of a new zone, the switches in the zone after are set to the desired state. This ensures that there is one zone of buffer time before the train arrives as the zone with the switch. Through previous testing, if the switches were activated upon entry of the zone, there will be cases (especially at higher speeds), that the switches don't activate on time.

= Collision prevention



= Servers

The following servers were introduced in TC2

== Trainstate server

The train state server maintains the single source of truth in terms of state of each train. This includes current speed setting, orientation, and position. A train position notifier runs sensor attribution on each sensor trigger and updates train position as well. Any desired changes to train state (such as setting the speed of a train) must be done through this server. This guarentees mutual exclusion. Trainstate server is also what can be used for a task that wishes to block until a train changes position. This is incredibly helpful for pathfinding algorithms.

== Reservation server

The reservation server keeps track of all reserved zones, as well as which train has reserved the zone. It has a simple reserve/unreserve interface that is idempotent (a train is allowed to reserve a zone many times).

== Trainterm (render) server
 
Since in our UI, each window is it's own task that is able to render on demand, we have multiple tasks attempting to print to the console at once. This can result in many visual bugs where tasks get interrupted in the middle of rendering in some text, especially when ANSI control sequences are involved. Thus, the render server was introduced to guarentee mutual exclusion on the cursor. Each window maintains it's own output buffer and writes into it. The ouput buffer is only flushed upon calling `w_flush()`, where the entire buffer is sent to the render server to be rendered atomically. The render server processes theses requests in FIFO order.

