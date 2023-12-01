s = """ 5: 3186181
 6: 2436675
 7: 1773222
 8: 1343748
 9: 1149748
10:  928405
11:  625165
12:  507750
13:  382140
14:  375001"""

speeds = [238, 294, 354, 391, 441, 484, 541, 587, 602, 608]

times = []
dists = []
for i, line in enumerate(s.split("\n")):
  stop_time = int(line.strip().split(" ")[-1])

  dist = 1045-(speeds[i]*stop_time*10**-6)
  dists.append(str(round(dist)))

  time = 2*dist*1000/speeds[i]
  times.append(str(round(time)))

print(", ".join(dists))
print(", ".join(times))
