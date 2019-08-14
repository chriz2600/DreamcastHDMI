
.SILENT:
all: build

build:
	gcc -std=c11 -o firmware-packer firmware-packer.c fastlz.c
	gcc -std=c11 -o firmware-unpacker firmware-unpacker.c fastlz.c

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

