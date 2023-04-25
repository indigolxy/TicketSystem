from random import *

def makeString(length):
    CharSet = "abcdefghijklmnopqrstuvwxyz"
    s = ""
    for i in range(length):
        s += choice(CharSet)
    return str(s)


mode = ["insert", "delete", "find"]

n = 10000
a = open("../../cmake-build-debug/test.txt", "w")
a.write(str(n))
a.write("\n")

strs = []

for i in range(n):
    m = randint(0, 2)
    if (m != 0 and len(strs) == 0):
        m = 0
    if (m == 0):
        s = makeString(10)
        strs.append(s)
    else:
        s = choice(strs)
    a.write(mode[m])
    a.write(" ")
    a.write(s)
    a.write(" ")
    if m != 2:
        a.write(str(randint(1, 1)))
    a.write("\n")
a.flush()
