#!env python3

#for x in range(859, 1400):
#    print(x / 858);

# 1,06993006993007
#   1,05893536121673

for vert in range(1053, 1900):
    for horiz in range(859, 1400):
        horiztest = (horiz / 858)
        verttest = (vert / 1052)
        ref = horiztest * verttest * 54
        if ref == 72.0:
            print("%.60f %.60f %f %f %f" % (horiztest, verttest, ref, horiz, vert));


# ./assets/test.py | awk '{ print length, $0 }' | sort -n | cut -d" " -f2- | less