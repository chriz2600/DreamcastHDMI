#!/usr/bin/env python3

#for x in range(859, 1400):
#    print(x / 858);

# 1,06993006993007
#   1,05893536121673

results = {}

for vert in range(1053, 1900):
    for horiz in range(859, 1400):
        horiztest = (horiz / 858)
        verttest = (vert / 1052)
        ref = horiztest * verttest * 54
        results[ref] = { 
                "horiztest": horiztest,
                "verttest": verttest,
                "ref": ref,
                "horiz": horiz,
                "vert": vert
            }

for res in results:
    data = results[res]
    print("%g MHz: %g %g (%.60f %.60f)" % (res, data["horiz"], data["vert"], data["horiztest"], data["verttest"]))

# ./assets/test.py | awk '{ print length($1), $0 }' | sort -n | cut -d" " -f2- | head -10
