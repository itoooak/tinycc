CFLAGS=-std=c11 -g -static

tinycc: tinycc.c

test: tinycc
	./test.sh

clean:
	rm tinycc tmp*

.PHONY: test clean
