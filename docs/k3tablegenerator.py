s1 = [(n*10, 1, n) for n in range(1, 21)]
s2 = [(n*23, 2, n) for n in range(1, 10)]
s3 = [(n*33, 3, n) for n in range(1, 7)]
s4 = [(n*71, 4, n) for n in range(1, 4)]

full = sorted(s1 + s2 + s3 + s4)

for i in full:
    arr = [str(i[1]+7)]
    for j in range(i[1] - 1):
        arr.append("")
    arr.append(str(i[0]))
    for j in range(4 - i[1]):
        arr.append("")
    arr.append(str(i[2]))
    print("[" + "], [".join(arr) + "],")
