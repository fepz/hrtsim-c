CC=gcc
BIN=./bin
CFLAGS=-Wall -Werror -g

PROGS=sim

.PHONY: all
all: $(PROGS)

%: %.c
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(PROGS)
