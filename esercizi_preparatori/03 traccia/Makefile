CC = gcc -Wall -g
LDFLAGS = -lpthread

all: main

main: common.h main.c
	$(CC) -o main main.c $(LDFLAGS)

.PHONY: clean
clean:
	killall -q main ; rm -f main build/*.o
