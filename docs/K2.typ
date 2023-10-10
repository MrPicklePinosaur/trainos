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
  title: "CS452 K2",
  authors: (
    "Joey Zhou (j438zhou, 20894170)",
    "Daniel Liu (d278liu, 20892282)",
  ),
  date: "October 5, 2023",
)

= Git Info

Gitlab repository name: `d278liu/trainos`

Commit SHA: `a5288f25043a52f392c8410d9b9bdc50a6526912`

= Kernel Features

== The Kernel Screen

The kernel begins by printing the TrainOS logo.
Just below the logo is a section labelled `TODAY'S ROLL`.
This is purely decorative and can be ignored.
#footnote[
  It's a gacha system which rolls a random schedule of UW courses, and gives each a rarity between 1-pip and 5-pip.
  It will reroll every time you boot up the kernel. See if you can get a 5-pip CS 452!
]

Below that section is the task list, prompted with the words `SELECT TASK TO RUN`.
Each task has a corresponding number, and you can input the number of a task to run it.
Run the task titled `K2` to run the RPS test, and run the task titled `K2Perf` to run the performance measurements.

== Message Passing

Our kernel implements the following primitives as described on the assignment page, with no changes to their signatures:

```c
int Send(int tid, const char *msg, int msglen, char *reply, int replylen)
int Receive(int *tid, char *msg, int msglen)
int Reply(int tid, void *reply, int replylen)
```

The scheduler properly blocks tasks that call Send or Receive.
They will not be run again until they receive a message.

== Name Server

The name server is created when the kernel boots up, as a child task of the init task.
It implements the following primitives as described on the assignment page, with no changes to their signatures:

```c
int RegisterAs(const char *name)
int WhoIs(const char *name)
```

== RPS

Clients can play RPS with the following functions:

```c
int Signup(Tid rps)
RPSResult Play(Tid rps, RPSMove move)
int Quit(Tid rps)
```

Some notes:
 - The `rps` parameter is the TID of the RPS server. This can be located using the name server.
 - `RPSResult` is a struct that stores the result of an RPS game, signifying a win, loss, draw, or that the opponent quit.
 - `RPSMove` is a struct that stores the move that the player wants to make.
 - `Signup` and `Quit` return a negative integer if there is an error, otherwise they return 0.
 - If one player quits from a game, the other player can still send in moves using Play(), but they will receive a result indicating that the opponent has quit.
 - Multiple games between different pairs of clients can be played at the same time.
 - After quitting, clients can sign up again to play another match. They may match up with a different opponent.

= Kernel Implementation

== Name Server

The name server database is currently implemented with a linked list.
Calling `RegisterAs()` adds the task to the linked list, and calling `WhoIs()` does a linear search for the requested task.
We expect that the number of named tasks should be low, so these linear operations shouldn't incur too large a cost.

== RPS

Rock paper scissors games are stored in a hash map with 32 buckets.
We chose 32 buckets because we expect the total number of RPS games running at any time to be lower than this.
The hash map's keys are the TIDs of the participating tasks; each game is stored twice under both TIDs.
When a player plays, the corresponding game is updated with their move.
If both players have moves recorded, the game is evaluated and the results are replied to each player.
Games are removed from the hash map when they are quit.

= RPS Tests

Running K2 will run three tests.
These tests will print their outputs in quick succession.
You are intended to scroll up in the console to view all of the outputs.

== Test 1

Test 1 showcases the basic functionality of RPS with a single game between two players.
Player 1 plays rock thrice before quitting, and player 2 plays scissors, paper, rock, paper, and finally scissors before quitting.
This is summarized in this table:

#table(
  columns: (auto, auto, auto, auto, auto),
  [*Move*], [*Player A*], [*Player B*], [*Player A Result*], [*Player B Result*],
  [1], [Rock], [Scissors], [Win], [Lose],
  [2], [Rock], [Paper], [Lose], [Win],
  [3], [Rock], [Rock], [Tie], [Tie],
  [4], [Quit], [Paper], [], [Opp. Quit],
  [5], [], [Scissors], [], [Opp. Quit],
  [6], [], [Quit], [], []
)

The print statements for test 1 should showcase the above results.
They should start with the following lines (although the player numbers may be different):
```
[RPS PLAYER 5] Requesting sign up
[RPS PLAYER 6] Requesting sign up
[RPS SERVER] Player 5 joined, no players in queue
[RPS SERVER] PLayer 6 joined, player 5 in queue, starting game
[RPS PLAYER 5] Successfully signed up
[RPS PLAYER 5] Playing move ROCK
[RPS PLAYER 6] Successfully signed up
[RPS PLAYER 6] Playing move SCISSORS
```

Note that all prints starting with `[RPS PLAYER X]` are done by player X's task, and all prints starting with `[RPS SERVER]` are done by the RPS server task.

== Test 2

Test 2 showcases the playing of multiple RPS games at the same time.
These tests use Player A and Player B from the test 1, though they are paired up in different ways.

One game should be a Player A vs Player B game. It should have the same results as Test 1.

Another game should be a Player A vs Player A game. It should tie 3 times in a row.

The final game should be a Player B vs Player B game. It should tie 5 times in a row.

The print statements should showcase the server handling signups, plays, and quits from the above three games at the same time.

== Test 3

Test 3 showcases how clients can play more RPS games even after quitting their first.
This test creates three players, each of whom will sign up, play rock, quit, sign up again, play scissors, then quit.

Though there are three players, only two players can play at once.
This causes their games to be offset in the following way:

#table(
  columns: (auto),
  inset: 0pt,
  table(
    columns: (10%, 30%, 30%, 30%),
    [*Step*], [*Player A*], [*Player B*], [*Player C*],
    [1], [Signup, waiting], [], [],
    [2], [], [Signup, matched with A], [],
    [3], [], [], [Signup, waiting],
  ),
  table(
    columns: (10%, 90%),
    [4], [Player A plays rock, player B plays rock, tie. Both quit.]
  ),
  table(
    columns: (10%, 30%, 30%, 30%),
    [5], [Signup, matched with C], [], [],
    [6], [], [Signup, waiting], [],
  ),
  table(
    columns: (10%, 90%),
    [7], [Player A plays scissors, player C plays rock, player C wins. Both quit.],
  ),
  table(
    columns: (10%, 30%, 30%, 30%),
    [8], [], [], [Signup, matched with B],
  ),
  table(
    columns: (10%, 90%),
    [9], [Player B plays scissors, player C plays scissors, tie. Both quit.],
  ),
)

= Performance Measurements

For each of the twelve tests, we create one task that sends and one task that receives.
Each task loops through 20 iterations of Send/Receive/Reply.
Since the variance on times is low, we believe that 20 is enough iterations to obtain a good average time.
As well, since our heap allocator is still incapable of reclaiming memory, we cannot increase this number by much without running out of memory.
#footnote[If you try running the performance test multiple in the same session, you can see what happens when our allocator runs out of memory.]

Some notes:
 - The timer is created in the sending task.
 - The timer starts just before Send() is called and stops just after we return from Send() (and thus have obtained a reply).

We use the following methodology to measure time:
```c
for (int i = 0; i < 20; i++) {
  timer_start();
  Send();
  timer_end();
}
```

We use this methodology because it allows us to measure variance.
As well, the overhead from starting and stopping the timer is negligible, which we know because when we tried this methodology:
```c
timer_start();
for (int i = 0; i < 20; i++) {
  Send();
}
timer_end();
```
we found that it was not consistently slower or faster than the other methodology.

