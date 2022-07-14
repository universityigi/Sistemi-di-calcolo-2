CC = gcc -Wall -g

all: search

search: search.c common.h
	$(CC) -o search search.c -lpthread

.PHONY: clean
clean:
	rm -f search
