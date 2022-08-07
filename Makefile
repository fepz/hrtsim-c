CC=gcc
BIN=./bin
CFLAGS=-Wall -Werror -g -lpthread -lrt

PROGS=sim

.PHONY: all
all: $(PROGS)

%: %.c
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(PROGS)
