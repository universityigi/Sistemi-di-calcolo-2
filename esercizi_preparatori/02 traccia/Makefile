CC = gcc -Wall -g
LDFLAGS = -lpthread

all: search

search: search.c common.h
	$(CC) -o search search.c $(LDFLAGS)

.PHONY: clean
clean:
	rm -f search
