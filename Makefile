CC = gcc
CFLAGS = -Wall -g

all: memgrind memtest test

memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

memtest: memtest.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

test: test.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c mymalloc.h
	$(CC) $(CFLAGS) -c $<

check: test
	for t in overlap reuse coalesce align toobig leak notmalloc offset double; do \
		./test $$t; echo "  exit status $$?"; \
	done

clean:
	rm -f *.o memgrind memtest test

.PHONY: all check clean
