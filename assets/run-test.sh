#!/bin/sh

cd $(dirname $0)

./test.py | awk '{ print length($1), $0 }' | sort -n | cut -d" " -f2- | head -10
