#!/usr/bin/env python3

#for x in range(859, 1400):
#    print(x / 858);

# 1,06993006993007
#   1,05893536121673

import argparse
from collections import OrderedDict

parser = argparse.ArgumentParser()
parser.add_argument("--width", help="width of input data (scaled)", type=int, default=858)
parser.add_argument("--height", help="height of input data (scaled)", type=int, default=1052)
parser.add_argument("--ref", help="reference clock", type=int, default=54)
parser.add_argument("--minwidth", help="minimum width", type=int, default=858)
parser.add_argument("--minheight", help="minimum height", type=int, default=1052)
parser.add_argument("--limit", help="limit results", type=int, default=20)
parser.add_argument("--is60Hz", help="reference to 60Hz", action='store_true')
args = parser.parse_args()

results = {}

HORIZ = args.width
VERT = args.height
REF = args.ref
if (args.is60Hz):
    REF = REF * 1.001

for vert in range(VERT + 1, VERT * 2):
    for horiz in range(HORIZ + 1, HORIZ * 2):
        horiztest = (horiz / HORIZ)
        verttest = (vert / VERT)
        ref = horiztest * verttest * REF
        results[ref] = { 
                "horiztest": horiztest,
                "verttest": verttest,
                "ref": ref,
                "horiz": horiz,
                "vert": vert
            }

results = OrderedDict(sorted(results.items()))

count = 0
print("Clock Horiz Vert")
for res in results:
    data = results[res]
    xres = res
    res2 = round(res, 2)
    if res2 == res:
        if data["horiz"] > args.minwidth and data["vert"] > args.minheight:
            print("%g MHz: %g %g" % (res2, data["horiz"], data["vert"]))
            count = count + 1
    if (count == args.limit):
        break

# ./assets/find-freq.py | awk '{ print length($1), $0 }' | sort -n | cut -d" " -f2- | head -10
