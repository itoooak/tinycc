CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

tinycc: $(OBJS)
	$(CC) -o tinycc $(OBJS) $(LDFLAGS)

$(OBJS): tinycc.h

test: tinycc
	./test.sh

clean:
	rm -f tinycc *.o *~ tmp*

.PHONY: test clean
