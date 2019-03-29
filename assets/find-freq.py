#!/usr/bin/env python3 -OO

#for x in range(859, 1400):
#    print(x / 858);

# 1,06993006993007
#   1,05893536121673

import argparse
from collections import OrderedDict
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--width", help="width of input data (scaled)", type=int, default=858)
parser.add_argument("--height", help="height of input data (scaled)", type=int, default=1052)
parser.add_argument("--ref", help="reference clock in MHz", type=int, default=54)
parser.add_argument("--minwidth", help="minimum width", type=int, default=858)
parser.add_argument("--minheight", help="minimum height", type=int, default=1052)
parser.add_argument("--maxwidth", help="minimum width", type=int, default=1716)
parser.add_argument("--maxheight", help="minimum height", type=int, default=1052)
parser.add_argument("--startfreq", help="frequency to start with in MHz", type=int, default=54)
parser.add_argument("--endfreq", help="frequency to end test with in MHz", type=int, default=54)
parser.add_argument("--limit", help="limit results", type=int, default=0)
parser.add_argument("--compensated", help="reference to freq / 1.001", action='store_true')
parser.add_argument("--precision", help="frequency precision (10->.1,100->.01 and so on)", type=int, default=10)
args = parser.parse_args()

results = {}

HORIZ = args.width
VERT = args.height
REF = (args.ref / args.width / args.height * 1.0)
if args.compensated:
    REF = REF * 1.001
REF = REF * 1000000
#REF = round(REF, 15)

# print("%.100g" % (REF))
# print("%.100g" % (108000000 / 1800 / 1000))

FREQ_MULT = args.precision
DIV = FREQ_MULT / 10
found = 0
idx = 0
reffreq = (args.ref / HORIZ) * 1000000 

print("+---------------------------------------------------------------------------+")

if args.compensated:
    print("| Compensation is done in PLL, treat reference for 2nd PLL as: %05.2f Hz     |" % (REF))
else:
    print("| No compensation done, try to achieve: %05.2f Hz                            |" % (REF))

print("+---------------------------------------------------------------------------+")
print("| frequency: %3.1f -> %3.1f MHz                                             |" % (args.startfreq, args.endfreq))
print("|     width:  %4i ->  %4i pixel                                           |" % (args.minwidth, args.maxwidth))
print("|    height:  %4i ->  %4i pixel                                           |" % (args.minheight, args.maxheight))
print("|  hreffreq:     %10.4f kHz                                             |" % (reffreq))
print("|  vreffreq:     %10.4f Hz                                              |" % (REF))
print("|  freqmult:           %4i                                                 |" % (FREQ_MULT))
print("+---------------------------------------------------------------------------+")

intermediate = {}

for freq in range(args.startfreq * FREQ_MULT, (args.endfreq * FREQ_MULT + 1)):
    if round(freq / FREQ_MULT, 1) == freq / FREQ_MULT:
        print("| processing: %06.3f MHz (matches: %03i)                                    |" % ((freq / FREQ_MULT), found), end='\r')
    for horiz in range(args.minwidth, args.maxwidth + 1):
        for vert in range(args.minheight, args.maxheight + 1):
            test = (freq / DIV) / horiz / vert
            test = test * 1000000 / (FREQ_MULT / DIV)
            test2 = (freq / FREQ_MULT) / horiz / vert
            test2 = test2 * 1000000
            #test = round(test, 15)
            # print();
            # print("%.100g %.100g %i %i" % (test, REF, test == REF, (freq / FREQ_MULT)))
            # print();
            if test == REF or test2 == REF:
                # print("%03i %06.3f / %04i / %04i" % (idx, freq / FREQ_MULT, horiz, vert))
                found = found + 1
                results[idx] = { 
                        "horiztest": horiz,
                        "verttest": vert,
                        "ref": (freq / FREQ_MULT),
                        "horiz": horiz,
                        "vert": vert
                    }
                idx = idx + 1

results = OrderedDict(sorted(results.items()))

limit = args.limit
if limit > 0:
    limit = found

count = 0
print("", end='\n')
print("+---------------------------------------------------------------------------+")
print("| Possible matches, please verify!                                          |")
print("+-------------------+-------+-------+-------------------------+-------------+")
print("| Clock   /  (real) |    f1 |    f2 | horiz freq /     (real) | buffer size |")
print("+-------------------+-------+-------+-------------------------+-------------+")
for res in results:
    data = results[res]
    xres = data['ref']
    res2 = round(xres, 3)
    res3 = res2
    if res2 == xres:
        if data["horiz"] >= args.minwidth and data["vert"] >= args.minheight:
            exact = ""
            extest = (round(xres, 3) - round(xres, 0))
            if (extest == 0 or extest == 0.25 or extest == 0.5 or extest == 0.75):
                exact = "<-"
            hfreq = (res2 * 1000000 / data["horiz"])
            hfreqc = hfreq
            if args.compensated:
                res3 = res3 / 1.001
                hfreqc = hfreqc / 1.001
            bufferneeded = round(abs((reffreq / hfreqc * 480) - 480), 0)
            print("| %6.3f / %6.3f | %5g | %5g | %10.4f / %10.4f | %11i | %s" % (res2, res3, data["horiz"], data["vert"], hfreq, hfreqc, bufferneeded, exact))
            count = count + 1
    if (count == limit):
        break
print("+-------------------+-------+-------+-------------------------+-------------+")

# 1080p variants with compensation
# ./assets/find-freq.py --width 1716 --height 1050 --minwidth 1920 --maxwidth 2300 --minheight 1080 --maxheight 1200 --startfreq 120 --endfreq 150 --compensated --ref 108
# ./assets/find-freq.py --width 1716 --height 1050 --minwidth 1990 --maxwidth 2300 --minheight 1100 --maxheight 1200 --startfreq 120 --endfreq 133 --compensated --ref 108 --precision 1000
# 960p with compensation
# ./assets/find-freq.py --width 1716 --height 1050 --minwidth 1600 --maxwidth 1850 --minheight 950 --maxheight 1100 --startfreq 105 --endfreq 110 --ref 108 --compensate
# 960p without compensation
# ./assets/find-freq.py --width 1716 --height 1050 --minwidth 1600 --maxwidth 1850 --minheight 950 --maxheight 1100 --startfreq 105 --endfreq 110 --ref 108
# 240p -> 960p
# ./assets/find-freq.py --width 1716 --height 1052 --minwidth 1600 --maxwidth 1850 --minheight 950 --maxheight 1100 --startfreq 105 --endfreq 110 --ref 108