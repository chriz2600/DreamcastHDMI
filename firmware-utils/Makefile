
.SILENT:
all: build

build:
	gcc -o firmware-packer firmware-packer.c fastlz.c
	gcc -o firmware-unpacker firmware-unpacker.c fastlz.c

clean:
	rm -f firmware-packer
	rm -f firmware-unpacker
	rm -f test.archive
	rm -f test.copy

install: test
	cp firmware-packer /usr/local/bin
	cp firmware-unpacker /usr/local/bin

test: build
	echo "Testing v1"
	echo "----------"
	./firmware-packer test.orig test.archive
	./firmware-unpacker test.archive test.copy
	diff test.orig test.copy || exit 1
	echo
	echo "Testing v2"
	echo "----------"
	./firmware-packer -v2 test.orig test.orig test.orig test.archive
	./firmware-unpacker -f1 test.archive test.copy
	diff test.orig test.copy || exit 1

