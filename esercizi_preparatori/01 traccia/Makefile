CC = gcc -Wall -g
LDFLAGS = -lpthread

all: multi_thread

multi_thread: common.h multi_thread.c
	$(CC) -o multi_thread multi_thread.c $(LDFLAGS)

.PHONY: clean

clean:
	rm -f multi_thread
