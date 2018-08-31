
.SILENT:
all: build

build:
	gcc -std=c11 -o firmware-packer -I../lib/FastLZ firmware-packer.c ../lib/FastLZ/fastlz.c
	gcc -std=c11 -o firmware-unpacker -I../lib/FastLZ firmware-unpacker.c ../lib/FastLZ/fastlz.c

clean:
	rm -f firmware-packer
	rm -f firmware-unpacker
	rm -f test.archive
	rm -f test.copy

test: build
	./firmware-packer test.orig test.archive
	./firmware-unpacker test.archive test.copy
	diff test.orig test.copy || exit 1
	cp test.archive firmware.dc

